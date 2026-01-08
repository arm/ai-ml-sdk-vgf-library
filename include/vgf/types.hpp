/*
 * SPDX-FileCopyrightText: Copyright 2023-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <limits>

namespace mlsdk::vgflib {

constexpr int32_t INT32_MIN_VALUE = std::numeric_limits<int32_t>::min();
constexpr int32_t INT32_MAX_VALUE = std::numeric_limits<int32_t>::max();
constexpr int64_t INT64_MIN_VALUE = std::numeric_limits<int64_t>::min();
constexpr uint32_t UINT32_MAX_VALUE = std::numeric_limits<uint32_t>::max();

constexpr int64_t CONSTANT_INVALID_SPARSITY_DIMENSION = INT64_MIN_VALUE;
constexpr int64_t CONSTANT_NOT_SPARSE_DIMENSION = -1;
static_assert(CONSTANT_NOT_SPARSE_DIMENSION > CONSTANT_INVALID_SPARSITY_DIMENSION);
constexpr uint32_t CONSTANT_INVALID_MRT_INDEX = UINT32_MAX_VALUE;

/// \brief FourCC value
struct FourCCValue {
    constexpr FourCCValue(char a, char b, char c, char d) : a(a), b(b), c(c), d(d) {}

    bool operator==(const FourCCValue &other) const {
        return a == other.a && b == other.b && c == other.c && d == other.d;
    }

    char a;
    char b;
    char c;
    char d;
};

/// \brief Generate a FourCC value from four chars
constexpr FourCCValue FourCC(char a, char b, char c, char d) { return {a, b, c, d}; }

/// \brief Type of module code
enum class ModuleType {
    COMPUTE,
    GRAPH,
};

/// \brief Category of resource as it relates to usage in the graph.
enum class ResourceCategory {
    INPUT,
    OUTPUT,
    INTERMEDIATE,
    CONSTANT,
};

/**
 * \brief VGF type that corresponds to a VkDescriptorType enum of the Vulkan API
 *
 * See ToDescriptorType and ToVkDescriptorType in vulkan_helpers.generated.hpp
 */
using DescriptorType = int32_t;

/**
 * \brief VGF type that corresponds to a VkFormat enum of the Vulkan API
 *
 * See ToFormatType and ToVkFormat in vulkan_helpers.generated.hpp
 */
using FormatType = int32_t;

/// \brief Value that corresponds to an Undefined VkFormat.
constexpr FormatType UndefinedFormat() { return 0; }

} // namespace mlsdk::vgflib
