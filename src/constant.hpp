/*
 * SPDX-FileCopyrightText: Copyright 2025-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace mlsdk::vgflib {

constexpr const char CONSTANT_SECTION_VERSION[8] = {'C', 'O', 'N', 'S', 'T', '0', '0', '\0'};
constexpr size_t CONSTANT_SECTION_VERSION_OFFSET = 0;
constexpr size_t CONSTANT_SECTION_VERSION_SIZE = 8;
static_assert(sizeof(CONSTANT_SECTION_VERSION) == CONSTANT_SECTION_VERSION_SIZE);

constexpr size_t CONSTANT_SECTION_COUNT_OFFSET = CONSTANT_SECTION_VERSION_OFFSET + CONSTANT_SECTION_VERSION_SIZE;
constexpr size_t CONSTANT_SECTION_COUNT_SIZE = 8;

constexpr size_t CONSTANT_SECTION_METADATA_OFFSET = CONSTANT_SECTION_COUNT_OFFSET + CONSTANT_SECTION_COUNT_SIZE;

struct ConstantMetaData_V00 {
    uint32_t mrtIndex{};
    int32_t sparsityDimension{static_cast<int32_t>(CONSTANT_NOT_SPARSE_DIMENSION)};
    uint64_t size{};
    uint64_t offset{};
};

constexpr size_t CONSTANT_SECTION_METADATA_MRT_INDEX_OFFSET = 0;
constexpr size_t CONSTANT_SECTION_METADATA_MRT_INDEX_SIZE = 4;

constexpr size_t CONSTANT_SECTION_METADATA_SPARSITY_DIMENSION_OFFSET =
    CONSTANT_SECTION_METADATA_MRT_INDEX_OFFSET + CONSTANT_SECTION_METADATA_MRT_INDEX_SIZE;
constexpr size_t CONSTANT_SECTION_METADATA_SPARSITY_DIMENSION_SIZE = 4;

constexpr size_t CONSTANT_SECTION_METADATA_SIZE_OFFSET =
    CONSTANT_SECTION_METADATA_SPARSITY_DIMENSION_OFFSET + CONSTANT_SECTION_METADATA_SPARSITY_DIMENSION_SIZE;
constexpr size_t CONSTANT_SECTION_METADATA_SIZE_SIZE = 8;

constexpr size_t CONSTANT_SECTION_METADATA_OFFSET_OFFSET =
    CONSTANT_SECTION_METADATA_SIZE_OFFSET + CONSTANT_SECTION_METADATA_SIZE_SIZE;
constexpr size_t CONSTANT_SECTION_METADATA_OFFSET_SIZE = 8;

static_assert(sizeof(ConstantMetaData_V00) % 8 == 0);
static_assert(sizeof(ConstantMetaData_V00) == sizeof(uint32_t) + sizeof(int32_t) + sizeof(uint64_t) + sizeof(uint64_t));

static_assert(offsetof(ConstantMetaData_V00, mrtIndex) == CONSTANT_SECTION_METADATA_MRT_INDEX_OFFSET,
              "ConstantMetaData mrtIndex field offset mismatched from spec.");
static_assert(offsetof(ConstantMetaData_V00, sparsityDimension) == CONSTANT_SECTION_METADATA_SPARSITY_DIMENSION_OFFSET,
              "ConstantMetaData sparsityDimension field offset mismatched from spec.");
static_assert(offsetof(ConstantMetaData_V00, size) == CONSTANT_SECTION_METADATA_SIZE_OFFSET,
              "ConstantMetaData size field offset mismatched from spec.");
static_assert(offsetof(ConstantMetaData_V00, offset) == CONSTANT_SECTION_METADATA_OFFSET_OFFSET,
              "ConstantMetaData offset field offset mismatched from spec.");
} // namespace mlsdk::vgflib
