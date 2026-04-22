/*
 * SPDX-FileCopyrightText: Copyright 2025-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf_updater.hpp"
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
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace mlsdk::vgf_updater {

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

std::vector<ModuleRef> extractModules(const HeaderDecoder &headerDecoder, const MemoryMap &mapped, Encoder &encoder) {
    auto moduleDecoder =
        CreateModuleTableDecoder(mapped.ptr(headerDecoder.GetModuleTableOffset()), headerDecoder.GetModuleTableSize());
    if (moduleDecoder == nullptr) {
        throw std::runtime_error("Module table could not be decoded safely");
    }
    const auto numModules = moduleDecoder->size();
    std::vector<ModuleRef> moduleRefs;
    moduleRefs.reserve(numModules);
    for (uint32_t i = 0; i < numModules; i++) {
        const auto moduleType = moduleDecoder->getModuleType(i);
        const std::string moduleName = std::string(moduleDecoder->getModuleName(i));
        const std::string entryPoint = std::string(moduleDecoder->getModuleEntryPoint(i));

        if (moduleDecoder->hasHLSLCode(i)) {
            const std::string_view moduleCode = moduleDecoder->getHLSLModuleCode(i);
            moduleRefs.push_back(
                encoder.AddModule(moduleType, moduleName, entryPoint, ShaderType::HLSL, std::string(moduleCode)));
        } else if (moduleDecoder->hasGLSLCode(i)) {
            const std::string_view moduleCode = moduleDecoder->getGLSLModuleCode(i);
            moduleRefs.push_back(
                encoder.AddModule(moduleType, moduleName, entryPoint, ShaderType::GLSL, std::string(moduleCode)));
        } else if (moduleDecoder->hasSPIRVCode(i)) {
            const auto moduleCode = moduleDecoder->getSPIRVModuleCode(i);
            const std::vector<uint32_t> moduleCodeData(moduleCode.begin(), moduleCode.end());
            moduleRefs.push_back(encoder.AddModule(moduleType, moduleName, entryPoint, moduleCodeData));
        } else if (moduleDecoder->isSPIRV(i)) {
            moduleRefs.push_back(encoder.AddModule(moduleType, moduleName, entryPoint));
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
    if (constantsSize == 0U) {
        return constantsByIndex;
    }

    const auto constantsOffset = headerDecoder.GetConstantsOffset();
    const auto parsedConstants = parseConstantSection(mapped.ptr(constantsOffset), constantsSize);
    for (const auto &segment : sequenceTable.mSegments) {
        for (const auto constantIdx : segment.mConstants) {
            if (constantsByIndex.find(constantIdx) != constantsByIndex.end()) {
                continue;
            }
            if (constantIdx >= parsedConstants.size()) {
                throw std::runtime_error("Invalid constant index " + std::to_string(constantIdx));
            }
            constantsByIndex.emplace(constantIdx, parsedConstants.at(constantIdx));
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

        const auto getPushConstRangeRef = [&encoder](const auto &range) {
            return encoder.AddPushConstRange(range.mStageFlags, range.mOffset, range.mSize);
        };
        std::vector<PushConstRangeRef> pushConstants{};
        pushConstants.reserve(segment.mPushConstantRanges.size());
        std::transform(segment.mPushConstantRanges.begin(), segment.mPushConstantRanges.end(),
                       std::back_inserter(pushConstants), getPushConstRangeRef);

        std::array<uint32_t, 3> dispatchShape{};
        if (segment.mDispatchShape.size() == 3) {
            std::copy(segment.mDispatchShape.begin(), segment.mDispatchShape.end(), dispatchShape.begin());
        }

        uint32_t descriptorIdx = 0;
        for (const auto &dscInfo : segment.mDescriptorSetInfos) {
            std::vector<BindingSlotRef> descriptorBindingSlotsRefs;

            for (const auto &slot : dscInfo.mBindings) {
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
            const auto descriptorPosition = static_cast<uint32_t>(descriptorIdx);
            const uint32_t encodedSetIndex =
                (dscInfo.mSetIndex == descriptorPosition) ? std::numeric_limits<uint32_t>::max() : dscInfo.mSetIndex;
            descriptorSetInfoRefs.push_back(encoder.AddDescriptorSetInfo(descriptorBindingSlotsRefs, encodedSetIndex));
            ++descriptorIdx;
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

void update(const std::string &inputPath, const std::string &outputPath) {
    MemoryMap mapped(inputPath);
    const auto headerDecoder =
        CreateHeaderDecoder(mapped.ptr(), static_cast<uint64_t>(HeaderSize()), static_cast<uint64_t>(mapped.size()));
    if (!headerDecoder) {
        throw std::runtime_error("Invalid VGF file: header or section verification failed");
    }

    if (headerDecoder->IsLatestVersion()) {
        std::cout << "VGF file is already at the latest version: " << static_cast<unsigned>(headerDecoder->GetMajor())
                  << "." << static_cast<unsigned>(headerDecoder->GetMinor()) << "."
                  << static_cast<unsigned>(headerDecoder->GetPatch()) << "\n";
        return;
    }

    auto encoder = CreateEncoder(headerDecoder->GetEncoderVulkanHeadersVersion());

    const auto moduleRefs = extractModules(*headerDecoder, mapped, *encoder);

    const auto resourceTable = parseModelResourceTable(mapped.ptr(headerDecoder->GetModelResourceTableOffset()),
                                                       headerDecoder->GetModelResourceTableSize());
    const auto resourceRefs = collectResources(resourceTable, *encoder);

    const auto modelSequenceTableOffset = headerDecoder->GetModelSequenceTableOffset();
    const auto modelSequenceTableSize = headerDecoder->GetModelSequenceTableSize();
    const auto sequenceTable = parseModelSequenceTable(mapped.ptr(modelSequenceTableOffset), modelSequenceTableSize);

    const auto constantsByIndex = decodeConstants(*headerDecoder, mapped, sequenceTable);

    const auto &[modelInputBindingSlots, modelOutputBindingSlots] =
        encodeSegments(sequenceTable, moduleRefs, resourceRefs, constantsByIndex, resourceTable, *encoder);

    const auto getName = [](const auto &bindingSlot) -> const std::string & { return bindingSlot.mName; };
    std::vector<std::string> inputNames;
    inputNames.reserve(sequenceTable.mInputs.size());
    std::transform(sequenceTable.mInputs.begin(), sequenceTable.mInputs.end(), std::back_inserter(inputNames), getName);

    std::vector<std::string> outputNames;
    outputNames.reserve(sequenceTable.mOutputs.size());
    std::transform(sequenceTable.mOutputs.begin(), sequenceTable.mOutputs.end(), std::back_inserter(outputNames),
                   getName);

    encoder->AddModelSequenceInputsOutputs(modelInputBindingSlots, inputNames, modelOutputBindingSlots, outputNames);

    encoder->Finish();
    writeOutput(outputPath, *encoder);
}
} // namespace mlsdk::vgf_updater
