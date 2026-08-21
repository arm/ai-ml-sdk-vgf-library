/*
 * SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf-utils/memory_map.hpp"
#include "vgf-utils/temp_folder.hpp"

#include <gtest/gtest.h>

#include <cstring>
#include <fstream>
#include <limits>
#include <vector>

namespace {

std::filesystem::path writeFile(const TempFolder &tempFolder, const std::vector<uint8_t> &data) {
    const auto path = tempFolder.relative("mapped.bin");
    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<const char *>(data.data()), static_cast<std::streamsize>(data.size()));
    return path;
}

} // namespace

TEST(MemoryMap, PreservesMappedFileContents) {
    const TempFolder tempFolder("memory_map");
    const std::vector<uint8_t> expectedData{0x10, 0x20, 0x30};
    const auto path = writeFile(tempFolder, expectedData);

    const MemoryMap mapped(path.string());

    EXPECT_EQ(mapped.size(), expectedData.size());
    EXPECT_EQ(std::memcmp(mapped.ptr(), expectedData.data(), expectedData.size()), 0);
}

TEST(MemoryMap, AcceptsSatisfiedRequiredAlignment) {
    const TempFolder tempFolder("memory_map");
    const auto path = writeFile(tempFolder, {0x01});

    const MemoryMap mapped(path.string(), 1);

    EXPECT_EQ(mapped.size(), 1);
}

TEST(MemoryMap, RejectsInvalidRequiredAlignment) {
    const TempFolder tempFolder("memory_map");
    const auto path = writeFile(tempFolder, {0x01});

    EXPECT_THROW(MemoryMap(path.string(), 0), std::invalid_argument);
    EXPECT_THROW(MemoryMap(path.string(), 3), std::invalid_argument);
}

TEST(MemoryMap, RejectsRequiredAlignmentNotMetByMapping) {
    const TempFolder tempFolder("memory_map");
    const auto path = writeFile(tempFolder, {0x01});
    constexpr size_t UNSUPPORTED_ALIGNMENT = size_t{1} << (std::numeric_limits<size_t>::digits - 1);

    EXPECT_THROW(MemoryMap(path.string(), UNSUPPORTED_ALIGNMENT), std::runtime_error);
}
