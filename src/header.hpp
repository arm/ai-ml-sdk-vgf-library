/*
 * SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "vgf/types.hpp"

#include <cstddef>
#include <cstdint>

namespace mlsdk::vgflib {

struct FormatVersion {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
};

struct SectionEntry {
    SectionEntry(uint64_t offset, uint64_t size) : offset(offset), size(size) {}
    uint64_t offset;
    uint64_t size;
};

static_assert(offsetof(FourCCValue, a) == 0);
static_assert(offsetof(FourCCValue, b) == 1);
static_assert(offsetof(FourCCValue, c) == 2);
static_assert(offsetof(FourCCValue, d) == 3);
static_assert(sizeof(FourCCValue) == 4);

// Deprecated value. Replaced by FourCC below
// TODO: remove when we move to V1.0 of VGF spec
constexpr uint32_t HEADER_MAGIC_VALUE_OLD = 0xF0E1D2C3;
constexpr FourCCValue HEADER_MAGIC_VALUE = FourCC('V', 'G', 'F', '1');
constexpr size_t HEADER_MAGIC_OFFSET = 0;
constexpr size_t HEADER_VK_HEADER_VERSION_OFFSET = 4;
constexpr size_t HEADER_VERSION_OFFSET = 8;
constexpr size_t HEADER_HEADER_SIZE_VALUE = 128;

constexpr size_t HEADER_FIRST_SECTION_OFFSET = 16;
constexpr size_t HEADER_SECOND_SECTION_OFFSET = HEADER_FIRST_SECTION_OFFSET + sizeof(SectionEntry);
constexpr size_t HEADER_THIRD_SECTION_OFFSET = HEADER_SECOND_SECTION_OFFSET + sizeof(SectionEntry);
constexpr size_t HEADER_FOURTH_SECTION_OFFSET = HEADER_THIRD_SECTION_OFFSET + sizeof(SectionEntry);

constexpr size_t HEADER_MODULE_SECTION_OFFSET = HEADER_FIRST_SECTION_OFFSET;
constexpr size_t HEADER_MODULE_SECTION_OFFSET_OFFSET = HEADER_MODULE_SECTION_OFFSET + offsetof(SectionEntry, offset);
constexpr size_t HEADER_MODULE_SECTION_SIZE_OFFSET = HEADER_MODULE_SECTION_OFFSET + offsetof(SectionEntry, size);

constexpr size_t HEADER_MODEL_SEQUENCE_SECTION_OFFSET = HEADER_SECOND_SECTION_OFFSET;
constexpr size_t HEADER_MODEL_SEQUENCE_SECTION_OFFSET_OFFSET =
    HEADER_MODEL_SEQUENCE_SECTION_OFFSET + offsetof(SectionEntry, offset);
constexpr size_t HEADER_MODEL_SEQUENCE_SECTION_SIZE_OFFSET =
    HEADER_MODEL_SEQUENCE_SECTION_OFFSET + offsetof(SectionEntry, size);

constexpr size_t HEADER_MODEL_RESOURCE_SECTION_OFFSET = HEADER_THIRD_SECTION_OFFSET;
constexpr size_t HEADER_MODEL_RESOURCE_SECTION_OFFSET_OFFSET =
    HEADER_MODEL_RESOURCE_SECTION_OFFSET + offsetof(SectionEntry, offset);
constexpr size_t HEADER_MODEL_RESOURCE_SECTION_SIZE_OFFSET =
    HEADER_MODEL_RESOURCE_SECTION_OFFSET + offsetof(SectionEntry, size);

constexpr size_t HEADER_CONSTANT_SECTION_OFFSET = HEADER_FOURTH_SECTION_OFFSET;
constexpr size_t HEADER_CONSTANT_SECTION_OFFSET_OFFSET =
    HEADER_CONSTANT_SECTION_OFFSET + offsetof(SectionEntry, offset);
constexpr size_t HEADER_CONSTANT_SECTION_SIZE_OFFSET = HEADER_CONSTANT_SECTION_OFFSET + offsetof(SectionEntry, size);

constexpr uint8_t HEADER_MAJOR_VERSION_VALUE = 0;
constexpr uint8_t HEADER_MINOR_VERSION_VALUE = 4;
constexpr uint8_t HEADER_PATCH_VERSION_VALUE = 0;

// This is a reminder to trigger removal of deprecated features on major version bump
// * HEADER_MAGIC_VALUE_OLD
static_assert(HEADER_MAJOR_VERSION_VALUE == 0);

struct Header {
    Header(const SectionEntry &moduleSection, const SectionEntry &sequenceSection, const SectionEntry &resourceSection,
           const SectionEntry &constantSection, uint16_t vkHeaderVersion)
        : vkHeaderVersion(vkHeaderVersion), moduleSection{moduleSection.offset, moduleSection.size},
          sequenceSection{sequenceSection.offset, sequenceSection.size},
          resourceSection{resourceSection.offset, resourceSection.size}, constantSection{constantSection.offset,
                                                                                         constantSection.size} {}

    const FourCCValue magic = HEADER_MAGIC_VALUE;
    const uint16_t vkHeaderVersion{0};
    const uint16_t reserved0{0};
    const FormatVersion version{HEADER_MAJOR_VERSION_VALUE, HEADER_MINOR_VERSION_VALUE, HEADER_PATCH_VERSION_VALUE};
    const uint8_t reserved1{0};
    const uint32_t reserved2{0};
    const SectionEntry moduleSection;
    const SectionEntry sequenceSection;
    const SectionEntry resourceSection;
    const SectionEntry constantSection;
    const uint64_t reserved3{0};
    const uint64_t reserved4{0};
    const uint64_t reserved5{0};
    const uint64_t reserved6{0};
    const uint64_t reserved7{0};
    const uint64_t reserved8{0};
};

static_assert(sizeof(Header) == HEADER_HEADER_SIZE_VALUE, "Header size mismatched from spec.");
static_assert(offsetof(Header, magic) == HEADER_MAGIC_OFFSET, "Magic field offset mismatched from spec.");
static_assert(offsetof(Header, vkHeaderVersion) == HEADER_VK_HEADER_VERSION_OFFSET,
              "VK_HEADER_VERSION field offset mismatched from spec.");
static_assert(offsetof(Header, version) == HEADER_VERSION_OFFSET, "Format Version field offset mismatched from spec.");
static_assert(offsetof(Header, moduleSection) == HEADER_MODULE_SECTION_OFFSET,
              "Header module section field offset mismatched from spec.");
static_assert(offsetof(Header, moduleSection.offset) == HEADER_MODULE_SECTION_OFFSET_OFFSET,
              "Header module section offset sub-field offset mismatched from spec.");
static_assert(offsetof(Header, moduleSection.size) == HEADER_MODULE_SECTION_SIZE_OFFSET,
              "Header module section size sub-field offset mismatched from spec.");
static_assert(offsetof(Header, sequenceSection) == HEADER_MODEL_SEQUENCE_SECTION_OFFSET,
              "Header model sequence section field offset mismatched from spec.");
static_assert(offsetof(Header, sequenceSection.offset) == HEADER_MODEL_SEQUENCE_SECTION_OFFSET_OFFSET,
              "Header model sequence section offset sub-field offset mismatched from spec.");
static_assert(offsetof(Header, sequenceSection.size) == HEADER_MODEL_SEQUENCE_SECTION_SIZE_OFFSET,
              "Header model sequence section size sub-field offset mismatched from spec.");
static_assert(offsetof(Header, resourceSection) == HEADER_MODEL_RESOURCE_SECTION_OFFSET,
              "Header model sequence section field offset mismatched from spec.");
static_assert(offsetof(Header, resourceSection.offset) == HEADER_MODEL_RESOURCE_SECTION_OFFSET_OFFSET,
              "Header model sequence section offset sub-field offset mismatched from spec.");
static_assert(offsetof(Header, resourceSection.size) == HEADER_MODEL_RESOURCE_SECTION_SIZE_OFFSET,
              "Header model sequence section size sub-field offset mismatched from spec.");
static_assert(offsetof(Header, constantSection) == HEADER_CONSTANT_SECTION_OFFSET,
              "Header constant section field offset mismatched from spec.");
static_assert(offsetof(Header, constantSection.offset) == HEADER_CONSTANT_SECTION_OFFSET_OFFSET,
              "Header constant section offset sub-field offset mismatched from spec.");
static_assert(offsetof(Header, constantSection.size) == HEADER_CONSTANT_SECTION_SIZE_OFFSET,
              "Header constant section size sub-field offset mismatched from spec.");
} // namespace mlsdk::vgflib
