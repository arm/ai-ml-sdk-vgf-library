/*
 * SPDX-FileCopyrightText: Copyright 2025-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "vgf/decoder.hpp"
#include <cstdint>
#include <vgf/types.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#define VGFLIB_VK_HELPERS // Avoid need to include Vulkan headers
#include <vgf/vulkan_helpers.generated.hpp>

namespace mlsdk::vgfutils {

struct BindingSlot {
    BindingSlot() = default;
    BindingSlot(uint32_t index, uint32_t binding, uint32_t mrtIndex)
        : mIndex(index), mBinding(binding), mMrtIndex(mrtIndex) {}

    uint32_t mIndex{0};
    uint32_t mBinding{0};
    uint32_t mMrtIndex{0};
};

struct Resource {
    Resource() = default;
    Resource(uint32_t index, mlsdk::vgflib::ResourceCategory category,
             std::optional<mlsdk::vgflib::DescriptorType> descriptorType, mlsdk::vgflib::VkFormat vkFormat,
             mlsdk::vgflib::DataView<int64_t> shape, mlsdk::vgflib::DataView<int64_t> stride)
        : mIndex(index), mCategory(category), mDescriptorType(descriptorType), mVkFormat(vkFormat),
          mShape(shape.begin(), shape.end()), mStride(stride.begin(), stride.end()) {}

    uint32_t mIndex{0};
    mlsdk::vgflib::ResourceCategory mCategory{mlsdk::vgflib::ResourceCategory::INPUT};
    std::optional<mlsdk::vgflib::DescriptorType> mDescriptorType{std::nullopt};
    mlsdk::vgflib::FormatType mVkFormat{mlsdk::vgflib::UndefinedFormat()};
    std::vector<int64_t> mShape;
    std::vector<int64_t> mStride;
};

struct PushConstantRange {
    PushConstantRange() = default;
    PushConstantRange(uint32_t index, uint32_t stageFlags, uint32_t offset, uint32_t size)
        : mIndex(index), mStageFlags(stageFlags), mOffset(offset), mSize(size) {}

    uint32_t mIndex{0};
    uint32_t mStageFlags{0};
    uint32_t mOffset{0};
    uint32_t mSize{0};
};

struct Segment {
    Segment() = default;
    Segment(uint32_t index, mlsdk::vgflib::ModuleType type, uint32_t moduleIndex, std::string_view &name,
            std::vector<BindingSlot> inputs, std::vector<BindingSlot> outputs,
            std::vector<std::vector<BindingSlot>> descriptorSetInfos, std::vector<PushConstantRange> pushConstantRanges,
            std::vector<uint32_t> constants, std::vector<uint32_t> dispatchShape)
        : mIndex(index), mType(type), mModuleIndex(moduleIndex), mName(name), mInputs(std::move(inputs)),
          mOutputs(std::move(outputs)), mDescriptorSetInfos(std::move(descriptorSetInfos)),
          mPushConstantRanges(std::move(pushConstantRanges)), mConstants(std::move(constants)),
          mDispatchShape(std::move(dispatchShape)) {}

    uint32_t mIndex{0};
    mlsdk::vgflib::ModuleType mType{mlsdk::vgflib::ModuleType::COMPUTE};
    uint32_t mModuleIndex{0};
    std::string mName;
    std::vector<BindingSlot> mInputs;
    std::vector<BindingSlot> mOutputs;
    std::vector<std::vector<BindingSlot>> mDescriptorSetInfos;
    std::vector<PushConstantRange> mPushConstantRanges;
    std::vector<uint32_t> mConstants;
    std::vector<uint32_t> mDispatchShape;
};

struct NamedBindingSlot {
    BindingSlot mBindingSlot;
    std::string mName;
};

struct ModelSequence {
    ModelSequence() = default;
    ModelSequence(std::vector<Segment> segments, std::vector<NamedBindingSlot> inputs,
                  std::vector<NamedBindingSlot> outputs)
        : mSegments(std::move(segments)), mInputs(std::move(inputs)), mOutputs(std::move(outputs)) {}

    std::vector<Segment> mSegments;
    std::vector<NamedBindingSlot> mInputs;
    std::vector<NamedBindingSlot> mOutputs;
};

struct Constant {
    Constant() = default;
    Constant(uint32_t index, uint32_t mrtIndex, int64_t sparsityDimension, const void *data = nullptr,
             size_t constantSize = {})
        : mIndex(index), mMrtIndex(mrtIndex), mSparsityDimension(sparsityDimension), mConstantData(data),
          mConstantSize(constantSize) {}

    uint32_t mIndex{0};
    uint32_t mMrtIndex{0};
    int64_t mSparsityDimension{-1};
    const void *mConstantData{};
    size_t mConstantSize{};
};

ModelSequence parseModelSequenceTable(const void *data, uint64_t size);

std::vector<Resource> parseModelResourceTable(const void *data, uint64_t size);

} // namespace mlsdk::vgfutils
