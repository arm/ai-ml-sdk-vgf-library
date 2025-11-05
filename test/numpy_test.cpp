/*
 * SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf-utils/numpy.hpp"
#include "vgf-utils/temp_folder.hpp"

#include <gtest/gtest.h>

#include <vector>

TEST(NumPy, Roundtrip) {
    TempFolder tempFolder("numpy_test");

    std::vector<int32_t> data = {42, 65, 76, 98, 106};
    std::vector<int64_t> shape = {static_cast<int64_t>(data.size())};
    mlsdk::vgfutils::numpy::DType type{'i', sizeof(int32_t)};

    std::string filename = tempFolder.relative("test.npy").string();

    mlsdk::vgfutils::numpy::DataPtr writePtr{reinterpret_cast<char *>(data.data()), shape, type};
    mlsdk::vgfutils::numpy::write(filename, writePtr);

    MemoryMap mmap{filename};
    auto readPtr = mlsdk::vgfutils::numpy::parse(mmap);

    ASSERT_TRUE(readPtr.shape.size() == shape.size());
    for (unsigned i = 0; i < shape.size(); i++) {
        ASSERT_TRUE(readPtr.shape[i] == shape[i]);
    }
    ASSERT_TRUE(readPtr.dtype.byteorder == type.byteorder);
    ASSERT_TRUE(readPtr.dtype.kind == type.kind);
    ASSERT_TRUE(readPtr.dtype.itemsize == type.itemsize);
    const int32_t *test = reinterpret_cast<const int32_t *>(readPtr.ptr);
    for (unsigned i = 0; i < data.size(); i++) {
        ASSERT_TRUE(test[i] == data[i]);
    }
}

TEST(NumPy, RoundtripCallbackWrite) {
    TempFolder tempFolder("numpy_callback_test");

    std::vector<int32_t> data = {42, 65, 76, 98, 106};
    std::vector<int64_t> shape = {static_cast<int64_t>(data.size())};
    mlsdk::vgfutils::numpy::DType type{'i', sizeof(int32_t)};

    std::string filename = tempFolder.relative("test.npy").string();

    mlsdk::vgfutils::numpy::write(filename, shape, type, [&](std::ostream &oss) {
        const char *ptr = reinterpret_cast<const char *>(data.data());
        for (unsigned j = 0; j < data.size() * sizeof(uint32_t); j++) {
            oss << ptr[j];
        }
        return data.size() * sizeof(uint32_t);
    });

    MemoryMap mmap{filename};
    auto readPtr = mlsdk::vgfutils::numpy::parse(mmap);

    ASSERT_TRUE(readPtr.shape.size() == shape.size());
    for (unsigned i = 0; i < shape.size(); i++) {
        ASSERT_TRUE(readPtr.shape[i] == shape[i]);
    }
    ASSERT_TRUE(readPtr.dtype.byteorder == type.byteorder);
    ASSERT_TRUE(readPtr.dtype.kind == type.kind);
    ASSERT_TRUE(readPtr.dtype.itemsize == type.itemsize);
    const int32_t *test = reinterpret_cast<const int32_t *>(readPtr.ptr);
    for (unsigned i = 0; i < data.size(); i++) {
        ASSERT_TRUE(test[i] == data[i]);
    }
}
