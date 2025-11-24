/*
 * SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf_converter.hpp"
#include "parse_vgf.hpp"
#include "vgf-utils/memory_map.hpp"
#include "vgf/decoder.hpp"
#include "vgf/encoder.hpp"
#include "vgf/types.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace mlsdk::vgf_converter {

using namespace vgflib;
using namespace vgfutils;

namespace {

ResourceRef encodeResource(const Resource &resource, Encoder &encoder) {
    switch (resource.mCategory) {
    case ResourceCategory::INPUT:
        if (!resource.mDescriptorType.has_value()) {
            throw std::runtime_error("Input resource missing descriptor type");
        }
        return encoder.AddInputResource(resource.mDescriptorType.value(), resource.mVkFormat, resource.mShape,
                                        resource.mStride);
    case ResourceCategory::OUTPUT:
        if (!resource.mDescriptorType.has_value()) {
            throw std::runtime_error("Output resource missing descriptor type");
        }
        return encoder.AddOutputResource(resource.mDescriptorType.value(), resource.mVkFormat, resource.mShape,
                                         resource.mStride);
    case ResourceCategory::INTERMEDIATE:
        if (!resource.mDescriptorType.has_value()) {
            throw std::runtime_error("Intermediate resource missing descriptor type");
        }
        return encoder.AddIntermediateResource(resource.mDescriptorType.value(), resource.mVkFormat, resource.mShape,
                                               resource.mStride);
    case ResourceCategory::CONSTANT:
        return encoder.AddConstantResource(resource.mVkFormat, resource.mShape, resource.mStride);
    }

    throw std::runtime_error("Unsupported resource category");
}

std::unique_ptr<HeaderDecoder> loadHeaderSafely(const MemoryMap &mapped) {
    auto headerDecoder = CreateHeaderDecoder(mapped.ptr());
    if (!headerDecoder || !headerDecoder->IsValid()) {
        throw std::runtime_error("Invalid VGF header, bad magic value");
    }

    if (!VerifyModuleTable(mapped.ptr(headerDecoder->GetModuleTableOffset()), headerDecoder->GetModuleTableSize())) {
        throw std::runtime_error("Invalid module table section");
    }

    if (!VerifyModelResourceTable(mapped.ptr(headerDecoder->GetModelResourceTableOffset()),
                                  headerDecoder->GetModelResourceTableSize())) {
        throw std::runtime_error("Invalid model resource table section");
    }

    if (!VerifyModelSequenceTable(mapped.ptr(headerDecoder->GetModelSequenceTableOffset()),
                                  headerDecoder->GetModelSequenceTableSize())) {
        throw std::runtime_error("Invalid model sequence table section");
    }

    if (!VerifyConstant(mapped.ptr(headerDecoder->GetConstantsOffset()), headerDecoder->GetConstantsSize())) {
        throw std::runtime_error("Invalid constant section");
    }

    return headerDecoder;
}

std::vector<ModuleRef> extractModules(const HeaderDecoder &headerDecoder, const MemoryMap &mapped, Encoder &encoder) {
    auto moduleDecoder = CreateModuleTableDecoder(mapped.ptr(headerDecoder.GetModuleTableOffset()));
    const auto numModules = moduleDecoder->size();
    std::vector<ModuleRef> moduleRefs;
    moduleRefs.reserve(numModules);
    for (uint32_t i = 0; i < numModules; i++) {
        const auto moduleCode = moduleDecoder->getModuleCode(i);
        if (moduleCode.begin() == moduleCode.end()) {
            moduleRefs.push_back(encoder.AddPlaceholderModule(moduleDecoder->getModuleType(i),
                                                              std::string(moduleDecoder->getModuleName(i)),
                                                              std::string(moduleDecoder->getModuleEntryPoint(i))));
        } else {
            const std::vector<uint32_t> moduleCodeData(moduleCode.begin(), moduleCode.end());
            moduleRefs.push_back(encoder.AddModule(moduleDecoder->getModuleType(i),
                                                   std::string(moduleDecoder->getModuleName(i)),
                                                   std::string(moduleDecoder->getModuleEntryPoint(i)), moduleCodeData));
        }
    }
    return moduleRefs;
}

std::vector<ResourceRef> collectResources(const std::vector<Resource> &resourceTable, Encoder &encoder) {
    std::vector<ResourceRef> resourceRefs;
    resourceRefs.reserve(resourceTable.size());
    std::transform(resourceTable.begin(), resourceTable.end(), std::back_inserter(resourceRefs),
                   [&](const auto &resource) { return encodeResource(resource, encoder); });
    return resourceRefs;
}

std::unordered_map<uint32_t, Constant> decodeConstants(const HeaderDecoder &headerDecoder, const MemoryMap &mapped,
                                                       const ModelSequence &sequenceTable) {
    std::unordered_map<uint32_t, Constant> constantsByIndex;
    const auto constantsSize = headerDecoder.GetConstantsSize();
    if (!constantsSize) {
        return constantsByIndex;
    }

    const auto constantsOffset = headerDecoder.GetConstantsOffset();
    const auto constantDecoder = CreateConstantDecoder(mapped.ptr(constantsOffset));
    for (const auto &segment : sequenceTable.mSegments) {
        for (const auto constantIdx : segment.mConstants) {
            if (constantsByIndex.find(constantIdx) != constantsByIndex.end()) {
                continue;
            }
            const auto constantView = constantDecoder->getConstant(constantIdx);
            constantsByIndex.emplace(constantIdx,
                                     Constant(constantIdx, constantDecoder->getConstantMrtIndex(constantIdx),
                                              constantDecoder->getConstantSparsityDimension(constantIdx),
                                              constantView.begin(), constantView.size()));
        }
    }

    return constantsByIndex;
}

auto encodeSegments(const ModelSequence &sequenceTable, const std::vector<ModuleRef> &moduleRefs,
                    const std::vector<ResourceRef> &resourceRefs,
                    const std::unordered_map<uint32_t, Constant> &constantsByIndex,
                    const std::vector<Resource> &resourceTable, Encoder &encoder) {

    std::pair<std::vector<BindingSlotRef>, std::vector<BindingSlotRef>> modelSequenceIO{};

    for (const auto &segment : sequenceTable.mSegments) {
        std::vector<BindingSlotRef> segmentInputBindingSlots;
        std::vector<BindingSlotRef> segmentOutputBindingSlots;
        std::vector<DescriptorSetInfoRef> descriptorSetInfoRefs;
        descriptorSetInfoRefs.reserve(segment.mDescriptorSetInfos.size());

        auto &[modelInputBindingSlots, modelOutputBindingSlots] = modelSequenceIO;

        std::vector<ConstantRef> constantRefs{};
        constantRefs.reserve(segment.mConstants.size());
        for (const auto constantIdx : segment.mConstants) {
            const auto constantIt = constantsByIndex.find(constantIdx);
            if (constantIt == constantsByIndex.end()) {
                throw std::runtime_error("Missing constant data for index " + std::to_string(constantIdx));
            }
            const auto &constantData = constantIt->second;
            const auto &resource = resourceTable[constantData.mMrtIndex];
            if (resource.mCategory != ResourceCategory::CONSTANT) {
                throw std::runtime_error("Constant resource category mismatch");
            }
            const auto constantResourceRef = resourceRefs[constantData.mMrtIndex];
            constantRefs.push_back(encoder.AddConstant(constantResourceRef, constantData.mConstantData,
                                                       constantData.mConstantSize, constantData.mSparsityDimension));
        }

        std::vector<PushConstRangeRef> pushConstants{};
        pushConstants.reserve(segment.mPushConstantRanges.size());
        for (const auto &pushConstantRange : segment.mPushConstantRanges) {
            pushConstants.push_back(encoder.AddPushConstRange(pushConstantRange.mStageFlags, pushConstantRange.mOffset,
                                                              pushConstantRange.mSize));
        }

        std::array<uint32_t, 3> dispatchShape{};
        if (segment.mDispatchShape.size() == 3) {
            std::copy(segment.mDispatchShape.begin(), segment.mDispatchShape.end(), dispatchShape.begin());
        }

        for (const auto &dsc : segment.mDescriptorSetInfos) {
            std::vector<BindingSlotRef> descriptorBindingSlotsRefs;

            for (const auto &slot : dsc) {
                const auto &resource = resourceTable[slot.mMrtIndex];
                const auto resourceRef = resourceRefs[slot.mMrtIndex];
                const auto binding = encoder.AddBindingSlot(slot.mBinding, resourceRef);
                descriptorBindingSlotsRefs.push_back(binding);

                if (resource.mCategory == ResourceCategory::INPUT) {
                    segmentInputBindingSlots.push_back(binding);
                    modelInputBindingSlots.push_back(binding);
                }

                if (resource.mCategory == ResourceCategory::OUTPUT) {
                    segmentOutputBindingSlots.push_back(binding);
                    modelOutputBindingSlots.push_back(binding);
                }
            }
            descriptorSetInfoRefs.push_back(encoder.AddDescriptorSetInfo(descriptorBindingSlotsRefs));
        }

        encoder.AddSegmentInfo(moduleRefs[segment.mModuleIndex], segment.mName, descriptorSetInfoRefs,
                               segmentInputBindingSlots, segmentOutputBindingSlots, constantRefs, dispatchShape,
                               pushConstants);
    }
    return modelSequenceIO;
}

void writeOutput(const std::string &outputPath, Encoder &encoder) {
    std::ofstream output;
    output.exceptions(std::ios::failbit | std::ios::badbit);
    try {
        output.open(outputPath.c_str(), std::ofstream::binary | std::ofstream::trunc);
        if (!encoder.WriteTo(output)) {
            throw std::runtime_error("Failed to write contents of updated VGF to " + outputPath);
        }
        output.close();
    } catch (const std::ios_base::failure &e) {
        throw std::runtime_error("I/O error writing '" + outputPath + "': " + std::string(e.what()));
    }
}

} // namespace

void convert(const std::string &inputPath, const std::string &outputPath) {
    MemoryMap mapped(inputPath);
    const auto headerDecoder = loadHeaderSafely(mapped);

    if (headerDecoder->IsLatestVersion()) {
        std::cout << "VGF file is already at the latest version: " << static_cast<unsigned>(headerDecoder->GetMajor())
                  << "." << static_cast<unsigned>(headerDecoder->GetMinor()) << "."
                  << static_cast<unsigned>(headerDecoder->GetPatch()) << "\n";
        return;
    }

    auto encoder = CreateEncoder(headerDecoder->GetEncoderVulkanHeadersVersion());

    const auto moduleRefs = extractModules(*headerDecoder, mapped, *encoder);

    const auto resourceTable = parseModelResourceTable(mapped.ptr(headerDecoder->GetModelResourceTableOffset()));
    const auto resourceRefs = collectResources(resourceTable, *encoder);

    const auto modelSequenceTableOffset = headerDecoder->GetModelSequenceTableOffset();
    const auto sequenceTable = parseModelSequenceTable(mapped.ptr(modelSequenceTableOffset));

    const auto constantsByIndex = decodeConstants(*headerDecoder, mapped, sequenceTable);

    const auto &[modelInputBindingSlots, modelOutputBindingSlots] =
        encodeSegments(sequenceTable, moduleRefs, resourceRefs, constantsByIndex, resourceTable, *encoder);

    std::vector<std::string> inputNames;
    inputNames.reserve(sequenceTable.mInputs.size());
    for (const auto &bindingSlot : sequenceTable.mInputs) {
        inputNames.push_back(bindingSlot.mName);
    }

    std::vector<std::string> outputNames;
    outputNames.reserve(sequenceTable.mOutputs.size());
    for (const auto &bindingSlot : sequenceTable.mOutputs) {
        outputNames.push_back(bindingSlot.mName);
    }

    encoder->AddModelSequenceInputsOutputs(modelInputBindingSlots, inputNames, modelOutputBindingSlots, outputNames);

    encoder->Finish();
    writeOutput(outputPath, *encoder);
}
} // namespace mlsdk::vgf_converter
