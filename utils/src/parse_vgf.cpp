/*
 * SPDX-FileCopyrightText: Copyright 2025-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "parse_vgf.hpp"

#include <memory>
#include <string_view>
#include <utility>
#include <vector>

namespace mlsdk::vgfutils {
using namespace vgflib;
namespace {

std::vector<BindingSlot> parseBindingSlots(const ModelSequenceTableDecoder &decoder,
                                           BindingSlotArrayHandle bindingSlotsHandle) {
    std::vector<BindingSlot> output;
    output.reserve(decoder.getBindingsSize(bindingSlotsHandle));
    for (uint32_t i = 0; i < decoder.getBindingsSize(bindingSlotsHandle); ++i) {
        output.emplace_back(i, decoder.getBindingSlotBinding(bindingSlotsHandle, i),
                            decoder.getBindingSlotMrtIndex(bindingSlotsHandle, i));
    }
    return output;
}

std::vector<std::string_view> parseNames(const ModelSequenceTableDecoder &decoder, NameArrayHandle namesHandle) {
    std::vector<std::string_view> output;
    output.reserve(decoder.getNamesSize(namesHandle));
    for (uint32_t i = 0; i < decoder.getNamesSize(namesHandle); ++i) {
        output.emplace_back(decoder.getName(namesHandle, i));
    }
    return output;
}

std::vector<PushConstantRange> parsePushConstantRanges(const ModelSequenceTableDecoder &decoder,
                                                       PushConstantRangeHandle handle) {
    std::vector<PushConstantRange> output;
    output.reserve(decoder.getPushConstRangesSize(handle));
    for (uint32_t i = 0; i < decoder.getPushConstRangesSize(handle); ++i) {
        output.emplace_back(i, decoder.getPushConstRangeStageFlags(handle, i),
                            decoder.getPushConstRangeOffset(handle, i), decoder.getPushConstRangeSize(handle, i));
    }
    return output;
}

std::vector<uint32_t> dataViewToVector(DataView<uint32_t> dataView) { return {dataView.begin(), dataView.end()}; }

} // namespace

std::vector<Resource> parseModelResourceTable(const void *const data, uint64_t size) {
    const auto decoder = CreateModelResourceTableDecoder(data, size);

    std::vector<Resource> resources;
    resources.reserve(decoder->size());
    for (uint32_t i = 0; i < decoder->size(); i++) {
        resources.emplace_back(i, decoder->getCategory(i), decoder->getDescriptorType(i), decoder->getVkFormat(i),
                               decoder->getTensorShape(i), decoder->getTensorStride(i));
    }
    return resources;
}

ModelSequence parseModelSequenceTable(const void *data, uint64_t size) {
    const auto decoder = CreateModelSequenceTableDecoder(data, size);

    const auto inputsHandle = decoder->getModelSequenceInputBindingSlotsHandle();
    std::vector<BindingSlot> inputs = parseBindingSlots(*decoder, inputsHandle);
    const auto outputsHandle = decoder->getModelSequenceOutputBindingSlotsHandle();
    std::vector<BindingSlot> outputs = parseBindingSlots(*decoder, outputsHandle);

    const auto inputNamesHandle = decoder->getModelSequenceInputNamesHandle();
    std::vector<std::string_view> inputNames = parseNames(*decoder, inputNamesHandle);

    const auto outputNamesHandle = decoder->getModelSequenceOutputNamesHandle();
    std::vector<std::string_view> outputNames = parseNames(*decoder, outputNamesHandle);

    auto merge = [](const std::vector<BindingSlot> &bindings, const std::vector<std::string_view> &names) {
        std::vector<NamedBindingSlot> output;
        output.reserve(bindings.size());
        for (size_t i = 0; i < bindings.size(); ++i) {
            std::string name = "";
            if (names.size() == bindings.size()) {
                name = names[i];
            }

            BindingSlot binding = bindings[i];

            output.push_back({binding, std::move(name)});
        }
        return output;
    };

    std::vector<NamedBindingSlot> namedInputs = merge(inputs, inputNames);
    std::vector<NamedBindingSlot> namedOutputs = merge(outputs, outputNames);

    std::vector<Segment> segments;
    for (uint32_t i = 0; i < decoder->modelSequenceTableSize(); ++i) {
        const auto segmentType = decoder->getSegmentType(i);
        const auto segmentModuleIndex = decoder->getSegmentModuleIndex(i);
        auto segmentName = decoder->getSegmentName(i);

        const auto segInputsHandle = decoder->getSegmentInputBindingSlotsHandle(i);
        std::vector<BindingSlot> segmentInputs = parseBindingSlots(*decoder, segInputsHandle);

        const auto segOutputsHandle = decoder->getSegmentOutputBindingSlotsHandle(i);
        std::vector<BindingSlot> segmentOutputs = parseBindingSlots(*decoder, segOutputsHandle);

        std::vector<std::vector<BindingSlot>> segmentDescriptorSetInfos;
        segmentDescriptorSetInfos.reserve(decoder->getSegmentDescriptorSetInfosSize(i));
        for (uint32_t j = 0; j < decoder->getSegmentDescriptorSetInfosSize(i); ++j) {
            const auto descSlotsHandle = decoder->getDescriptorBindingSlotsHandle(i, j);
            segmentDescriptorSetInfos.emplace_back(parseBindingSlots(*decoder, descSlotsHandle));
        }

        const auto pcrHandle = decoder->getSegmentPushConstRange(i);
        std::vector<PushConstantRange> segmentPushConstantRange = parsePushConstantRanges(*decoder, pcrHandle);

        const DataView<uint32_t> segmentConstants = decoder->getSegmentConstantIndexes(i);
        const DataView<uint32_t> segmentDispatchShape = decoder->getSegmentDispatchShape(i);

        segments.emplace_back(i, segmentType, segmentModuleIndex, segmentName, std::move(segmentInputs),
                              std::move(segmentOutputs), std::move(segmentDescriptorSetInfos),
                              std::move(segmentPushConstantRange), dataViewToVector(segmentConstants),
                              dataViewToVector(segmentDispatchShape));
    }

    return {std::move(segments), std::move(namedInputs), std::move(namedOutputs)};
}

} // namespace mlsdk::vgfutils
