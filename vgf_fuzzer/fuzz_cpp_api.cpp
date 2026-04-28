/*
 * SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "fuzzers.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <vector>

#include "vgf/decoder.hpp"
#include "vgf/encoder.hpp"

using namespace mlsdk::vgflib;

void FuzzCppDecoders(const uint8_t *data, size_t size) {
    (void)CreateHeaderDecoder(data, static_cast<uint64_t>(HeaderSize()), size);
    (void)CreateModuleTableDecoder(data, size);
    (void)CreateModelResourceTableDecoder(data, size);
    (void)CreateModelSequenceTableDecoder(data, size);
    (void)CreateConstantDecoder(data, size);
}

void FuzzCppDecoderAccessors(const uint8_t *data, size_t size) {
    auto header = CreateHeaderDecoder(data, static_cast<uint64_t>(HeaderSize()), size);
    if (!header) {
        return;
    }

    header->IsValid();
    header->CheckVersion();
    header->IsLatestVersion();

    header->GetVersion();
    header->GetMajor();
    header->GetMinor();
    header->GetPatch();

    header->GetEncoderVulkanHeadersVersion();

    /**************************************************************************/
    const uint64_t modOffset = header->GetModuleTableOffset();
    const uint64_t modSize = header->GetModuleTableSize();
    auto modDec = CreateModuleTableDecoder(data + static_cast<size_t>(modOffset), modSize);
    if (modDec) {
        const auto count = modDec->size();
        if (count > 0) {
            const auto idx = static_cast<uint32_t>(std::min<size_t>(count - 1, UINT32_MAX));
            modDec->getModuleType(idx);
            modDec->getModuleName(idx);
            modDec->getModuleEntryPoint(idx);
            modDec->isSPIRV(idx);
            modDec->hasSPIRVCode(idx);
            modDec->isGLSL(idx);
            modDec->hasGLSLCode(idx);
            modDec->isHLSL(idx);
            modDec->hasHLSLCode(idx);
            modDec->getSPIRVModuleCode(idx);
            modDec->getGLSLModuleCode(idx);
            modDec->getHLSLModuleCode(idx);
        }
    }

    /**************************************************************************/
    const uint64_t mrtOffset = header->GetModelResourceTableOffset();
    const uint64_t mrtSize = header->GetModelResourceTableSize();
    auto mrtDec = CreateModelResourceTableDecoder(data + static_cast<size_t>(mrtOffset), mrtSize);
    if (mrtDec) {
        const auto count = mrtDec->size();
        if (count > 0) {
            const auto idx = static_cast<uint32_t>(std::min<size_t>(count - 1, UINT32_MAX));
            mrtDec->getDescriptorType(idx);
            const auto aliasGroupId = mrtDec->getAliasGroupId(idx);
            if (aliasGroupId.has_value()) {
                aliasGroupId.value();
            }
            mrtDec->getVkFormat(idx);
            mrtDec->getCategory(idx);
            mrtDec->getTensorShape(idx);
            mrtDec->getTensorStride(idx);
            const auto samplerConfigHandle = mrtDec->getSamplerConfigHandle(idx);
            if (samplerConfigHandle != nullptr) {
                mrtDec->getSamplerConfigMinFilter(samplerConfigHandle);
                mrtDec->getSamplerConfigMagFilter(samplerConfigHandle);
                mrtDec->getSamplerConfigAddressModeU(samplerConfigHandle);
                mrtDec->getSamplerConfigAddressModeV(samplerConfigHandle);
                mrtDec->getSamplerConfigBorderColor(samplerConfigHandle);
            }
        }
    }

    /**************************************************************************/
    const uint64_t constOffset = header->GetConstantsOffset();
    const uint64_t constSize = header->GetConstantsSize();
    auto constDec = CreateConstantDecoder(data + static_cast<size_t>(constOffset), constSize);
    if (constDec) {
        const auto count = constDec->size();
        if (count > 0) {
            const auto idx = static_cast<uint32_t>(std::min<size_t>(count - 1, UINT32_MAX));
            constDec->getConstantMrtIndex(idx);
            constDec->isSparseConstant(idx);
            constDec->getConstantSparsityDimension(idx);
            constDec->getConstant(idx);
        }
    }

    /**************************************************************************/
    const uint64_t seqOffset = header->GetModelSequenceTableOffset();
    const uint64_t seqSize = header->GetModelSequenceTableSize();
    auto seqDec = CreateModelSequenceTableDecoder(data + static_cast<size_t>(seqOffset), seqSize);
    if (seqDec) {
        const auto count = seqDec->modelSequenceTableSize();
        if (count > 0) {
            const auto idx = static_cast<uint32_t>(std::min<size_t>(count - 1, UINT32_MAX));
            const auto descCount = seqDec->getSegmentDescriptorSetInfosSize(idx);
            if (descCount > 0) {
                seqDec->getSegmentDescriptorSetIndex(idx, 0);
                const auto dIdx = static_cast<uint32_t>(std::min<size_t>(descCount - 1, UINT32_MAX));
                seqDec->getSegmentDescriptorSetIndex(idx, dIdx);
            }
            seqDec->getSegmentConstantIndexes(idx);
            seqDec->getSegmentType(idx);
            seqDec->getSegmentName(idx);
            seqDec->getSegmentModuleIndex(idx);
            seqDec->getSegmentDispatchShape(idx);

            auto descSlots = seqDec->getDescriptorBindingSlotsHandle(idx, 0);
            if (descSlots) {
                const auto bindCount = seqDec->getBindingsSize(descSlots);
                if (bindCount > 0) {
                    const auto bIdx = static_cast<uint32_t>(std::min<size_t>(bindCount - 1, UINT32_MAX));
                    seqDec->getBindingSlotBinding(descSlots, bIdx);
                    seqDec->getBindingSlotMrtIndex(descSlots, bIdx);
                }
            }

            auto segInputs = seqDec->getSegmentInputBindingSlotsHandle(idx);
            if (segInputs) {
                seqDec->getBindingsSize(segInputs);
            }
            auto segOutputs = seqDec->getSegmentOutputBindingSlotsHandle(idx);
            if (segOutputs) {
                seqDec->getBindingsSize(segOutputs);
            }

            auto modelInputs = seqDec->getModelSequenceInputBindingSlotsHandle();
            if (modelInputs) {
                seqDec->getBindingsSize(modelInputs);
            }
            auto modelOutputs = seqDec->getModelSequenceOutputBindingSlotsHandle();
            if (modelOutputs) {
                seqDec->getBindingsSize(modelOutputs);
            }

            auto nameInputs = seqDec->getModelSequenceInputNamesHandle();
            if (nameInputs) {
                auto namesInputsSize = seqDec->getNamesSize(nameInputs);
                if (namesInputsSize > 0) {
                    seqDec->getName(nameInputs, 0);
                }
            }

            auto nameOutputs = seqDec->getModelSequenceOutputNamesHandle();
            if (nameOutputs) {
                auto namesOutputsSize = seqDec->getNamesSize(nameOutputs);
                if (namesOutputsSize > 0) {
                    seqDec->getName(nameOutputs, 0);
                }
            }

            auto pcrh = seqDec->getSegmentPushConstRange(idx);
            if (pcrh) {
                const auto pcrCount = seqDec->getPushConstRangesSize(pcrh);
                if (pcrCount > 0) {
                    const auto pIdx = static_cast<uint32_t>(std::min<size_t>(pcrCount - 1, UINT32_MAX));
                    seqDec->getPushConstRangeStageFlags(pcrh, pIdx);
                    seqDec->getPushConstRangeOffset(pcrh, pIdx);
                    seqDec->getPushConstRangeSize(pcrh, pIdx);
                }
            }
        }
    }
}

void FuzzCppEncoderSmoke(const uint8_t *data, size_t size) {
    const auto vkHeaderVersion = size >= sizeof(uint16_t) ? static_cast<uint16_t>(data[0] | (data[1] << 8)) : 0;
    constexpr DescriptorType combinedImageSamplerDescriptorType = 1;
    constexpr uint32_t samplerMinFilter = 0;
    constexpr uint32_t samplerMagFilter = 1;
    constexpr uint32_t samplerAddressModeU = 2;
    constexpr uint32_t samplerAddressModeV = 3;
    constexpr uint32_t samplerBorderColor = 4;
    auto encoder = CreateEncoder(vkHeaderVersion);
    if (encoder == nullptr) {
        return;
    }

    const std::vector<uint32_t> spirv{0x07230203, 0x00010000};
    const auto spirvModule = encoder->AddModule(ModuleType::COMPUTE, "spirv_module", "main", spirv);
    (void)encoder->AddModule(ModuleType::GRAPH, "glsl_module", "main", ShaderType::GLSL, "void main(){}");
    (void)encoder->AddModule(ModuleType::COMPUTE, "hlsl_module", "main", ShaderType::HLSL,
                             "[numthreads(1,1,1)] void main(){}");

    const std::vector<int64_t> shape{1, 2};
    const std::vector<int64_t> strides{2, 1};
    const auto inputResource = encoder->AddInputResource(combinedImageSamplerDescriptorType, 0, shape, strides);
    encoder->AddSamplerConfig(inputResource, samplerMinFilter, samplerMagFilter, samplerAddressModeU,
                              samplerAddressModeV, samplerBorderColor);
    const auto outputResource = encoder->AddOutputResource(0, 0, shape, {});
    const auto intermediateResource = encoder->AddIntermediateResource(0, 0, shape, {});
    const auto constantResource = encoder->AddConstantResource(0, shape, {});

    const std::vector<uint8_t> constantData{1, 2, 3, 4};
    const auto constant = encoder->AddConstant(constantResource, constantData.data(), constantData.size());

    const auto inputBinding = encoder->AddBindingSlot(0, inputResource);
    const auto outputBinding = encoder->AddBindingSlot(1, outputResource);
    const auto intermediateBinding = encoder->AddBindingSlot(2, intermediateResource);
    const auto constantBinding = encoder->AddBindingSlot(3, constantResource);

    const auto descriptor =
        encoder->AddDescriptorSetInfo({inputBinding, outputBinding, intermediateBinding, constantBinding}, 0);
    const auto legacyDescriptor = encoder->AddDescriptorSetInfo();
    const auto pushConstRange = encoder->AddPushConstRange(1, 0, 4);
    const std::array<uint32_t, 3> dispatchShape{1, 1, 1};
    encoder->AddSegmentInfo(spirvModule, "segment", {descriptor, legacyDescriptor}, {inputBinding}, {outputBinding},
                            {constant}, dispatchShape, {pushConstRange});
    encoder->AddModelSequenceInputsOutputs({inputBinding}, {"input"}, {outputBinding}, {"output"});
    encoder->Finish();

    std::stringstream output;
    (void)encoder->WriteTo(output);
}
