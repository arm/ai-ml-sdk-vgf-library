/*
 * SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <numeric>
#include <string>
#include <vector>

namespace mlsdk::vgfutils::numpy {

namespace {
inline bool is_little_endian() {
    uint16_t num = 1;
    return reinterpret_cast<uint8_t *>(&num)[1] == 0;
}

inline char get_endian_char(uint64_t size) { return size < 2 ? '|' : is_little_endian() ? '<' : '>'; }

inline uint64_t size_of(const std::vector<int64_t> &shape, const uint64_t &itemsize) {
    return std::accumulate(shape.begin(), shape.end(), itemsize, std::multiplies<uint64_t>());
}

inline bool isPow2(uint32_t value) { return ((value & (~(value - 1))) == value); }

} // namespace

char numpyTypeEncoding(const std::string &numeric);

uint32_t elementSizeFromBlockSize(uint32_t blockSize);

void write(const std::string &filename, const char *ptr, const std::vector<int64_t> &shape, const char kind,
           const uint64_t &itemsize);

} // namespace mlsdk::vgfutils::numpy
