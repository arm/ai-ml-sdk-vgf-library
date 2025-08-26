/*
 * SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "numpy.hpp"

#include <array>
#include <fstream>
#include <sstream>

namespace mlsdk::vgfutils::numpy {

namespace {

constexpr std::array<char, 6> numpy_magic_bytes = {'\x93', 'N', 'U', 'M', 'P', 'Y'};

std::string shape_to_str(const std::vector<int64_t> &shape) {
    std::stringstream shape_ss;
    shape_ss << "(";

    if (shape.size() == 0) {
        // nothing to do here
    } else if (shape.size() == 1) {
        shape_ss << std::to_string(shape[0]) << ",";
    } else {
        for (uint32_t i = 0; i < shape.size(); ++i) {
            shape_ss << std::to_string(shape[i]);
            if (i != shape.size() - 1)
                shape_ss << ", ";
        }
    }
    shape_ss << ")";
    return shape_ss.str();
}

void write_header(std::ostream &out, const std::vector<int64_t> &shape, const std::string &dtype) {
    std::stringstream header_dict;
    header_dict << "{";
    header_dict << "'descr': '" << dtype << "',";
    header_dict << "'fortran_order': False,";
    header_dict << "'shape': " << shape_to_str(shape);
    header_dict << "}";

    // calculate padding len
    std::string header_str = header_dict.str();
    size_t padding_len = 16 - ((10 + header_str.size() + 1) % 16);
    header_str += std::string(padding_len, ' ') + '\n';

    // write magic string
    out.write(numpy_magic_bytes.data(), numpy_magic_bytes.size());

    // write version and HEADER_LEN
    size_t header_len = header_str.size();
    bool use_version_2 = header_len > 65535;

    if (use_version_2) {
        out.put(static_cast<char>(0x02));
        out.put(static_cast<char>(0x00));
        out.put(static_cast<char>(header_len & 0xFF));
        out.put(static_cast<char>((header_len >> 8) & 0xFF));
        out.put(static_cast<char>((header_len >> 16) & 0xFF));
        out.put(static_cast<char>((header_len >> 24) & 0xFF));
    } else {
        out.put(static_cast<char>(0x01));
        out.put(static_cast<char>(0x00));
        out.put(static_cast<char>(header_len & 0xFF));
        out.put(static_cast<char>((header_len >> 8) & 0xFF));
    }

    // write header dict
    out.write(header_str.c_str(), std::streamsize(header_str.size()));
}

} // namespace

char numpyTypeEncoding(const std::string &numeric) {
    if (numeric == "BOOL") {
        return 'b';
    }
    if (numeric == "SINT" || numeric == "SNORM") {
        return 'i';
    }
    if (numeric == "UINT" || numeric == "UNORM") {
        return 'u';
    }
    if (numeric == "SFLOAT" || numeric == "UFLOAT") {
        return 'f';
    }

    throw std::runtime_error("Unable to classify NumPy encoding");
}

uint32_t elementSizeFromBlockSize(uint32_t blockSize) {
    if (isPow2(blockSize)) {
        return blockSize;
    }
    // Round up to next power of 2
    uint32_t countBits = 1; // start with offset to select next power of 2
    while (blockSize >>= 1) {
        countBits++;
    }
    return 1 << countBits;
}

void write(const std::string &filename, const char *ptr, const std::vector<int64_t> &shape, const char kind,
           const uint64_t &itemsize) {

    char byteorder = get_endian_char(itemsize);
    std::string dtype = std::string(1, byteorder) + kind + std::to_string(itemsize);

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("cannot open " + filename);
    }

    // write npy header to file
    write_header(file, shape, dtype);

    // write data to file
    file.write(reinterpret_cast<const char *>(ptr), std::streamsize(size_of(shape, itemsize)));
    file.close();
}

} // namespace mlsdk::vgfutils::numpy
