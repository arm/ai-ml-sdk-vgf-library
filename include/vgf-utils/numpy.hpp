/*
 * SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "memory_map.hpp"

#include <cstdint>
#include <functional>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace mlsdk::vgfutils::numpy {

struct DType {
    char byteorder{'\0'};
    char kind{'\0'};
    uint64_t itemsize{0};

    DType() = default;
    DType(char kind, uint64_t itemsize, char byteorder) : byteorder(byteorder), kind(kind), itemsize(itemsize) {}
    DType(char kind, uint64_t itemsize);
};

struct DataPtr {
    const char *ptr = nullptr;
    std::vector<int64_t> shape = {};
    DType dtype = {};

    DataPtr() = default;
    DataPtr(const char *ptr, const std::vector<int64_t> &shape, const DType &dtype)
        : ptr(ptr), shape(shape), dtype(dtype){};

    uint64_t size() const;
};

char numpyTypeEncoding(std::string_view numeric);

uint32_t elementSizeFromBlockSize(uint32_t blockSize);

DataPtr parse(const MemoryMap &mapped);

void write(const std::string &filename, const DataPtr &dataPtr);

void write(const std::string &filename, const std::vector<int64_t> &shape, const DType &dtype,
           std::function<uint64_t(std::ostream &)> &&callback);

void write(const std::string &filename, const char *ptr, const std::vector<int64_t> &shape, const char kind,
           const uint64_t &itemsize);

} // namespace mlsdk::vgfutils::numpy
