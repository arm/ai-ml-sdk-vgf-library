/*
 * SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "fuzzers.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "vgf/decoder.hpp"

using namespace mlsdk::vgflib;

void FuzzCppDecoders(const uint8_t *data, size_t size) {
    (void)CreateHeaderDecoder(data, size);
    (void)CreateModuleTableDecoder(data, size);
    (void)CreateModelResourceTableDecoder(data, size);
    (void)CreateModelSequenceTableDecoder(data, size);
    (void)CreateConstantDecoder(data, size);
}

void FuzzCppDecoderAccessors(const uint8_t *data, size_t size) {
    auto header = CreateHeaderDecoder(data, size);
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
            if (modDec->hasSPIRV(idx)) {
                modDec->getModuleCode(idx);
            }
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
            mrtDec->getVkFormat(idx);
            mrtDec->getCategory(idx);
            mrtDec->getTensorShape(idx);
            mrtDec->getTensorStride(idx);
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
            seqDec->getSegmentDescriptorSetInfosSize(idx);
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
