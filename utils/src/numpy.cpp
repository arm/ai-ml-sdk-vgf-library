/*
 * SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf-utils/numpy.hpp"

#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>

namespace mlsdk::vgfutils::numpy {

namespace {

constexpr std::array<char, 6> numpyMagicBytes = {'\x93', 'N', 'U', 'M', 'P', 'Y'};

bool isLittleEndian() {
    uint16_t num = 1;
    return reinterpret_cast<uint8_t *>(&num)[1] == 0;
}

char getEndianChar(uint64_t size) { return size < 2 ? '|' : isLittleEndian() ? '<' : '>'; }

uint64_t sizeOf(const std::vector<int64_t> &shape, const uint64_t &itemsize) {
    return std::accumulate(shape.begin(), shape.end(), itemsize, std::multiplies<uint64_t>());
}

bool isPow2(uint32_t value) { return ((value & (~(value - 1))) == value); }

std::string shapeToStr(const std::vector<int64_t> &shape) {
    std::stringstream ss;
    ss << "(";

    if (shape.size() == 0) {
        // nothing to do here
    } else if (shape.size() == 1) {
        ss << std::to_string(shape[0]) << ",";
    } else {
        for (uint32_t i = 0; i < shape.size(); ++i) {
            ss << std::to_string(shape[i]);
            if (i != shape.size() - 1)
                ss << ", ";
        }
    }
    ss << ")";
    return ss.str();
}

std::string dtypeToStr(const DType &dtype) {
    return std::string(1, dtype.byteorder) + dtype.kind + std::to_string(dtype.itemsize);
}

std::vector<int64_t> strToShape(const std::string &shapeStr) {
    std::stringstream ss(shapeStr);
    std::string token;
    std::vector<int64_t> shape;

    while (std::getline(ss, token, ',')) {
        token.erase(0, token.find_first_not_of(' '));
        token.erase(token.find_last_not_of(' ') + 1);

        try {
            shape.push_back(std::stoull(token));
        } catch (const std::exception &e) {
            throw std::runtime_error(std::string("invalid shape: ") + e.what());
        }
    }
    return shape;
}

DType getDtype(const std::string &dict) {
    size_t descrStart = dict.find("'descr':");
    if (descrStart == std::string::npos)
        throw std::runtime_error("missing 'descr' field in header");

    descrStart += 8;
    size_t valueStart = dict.find('\'', descrStart);
    size_t valueEnd = dict.find('\'', valueStart + 1);
    if (valueStart == std::string::npos || valueEnd == std::string::npos)
        throw std::runtime_error("invalid 'descr' format in header");

    std::string descrValue = dict.substr(valueStart + 1, valueEnd - valueStart - 1);
    if (descrValue.size() < 3)
        throw std::runtime_error("invalid 'descr' string");

    char byteorder = descrValue[0];
    char kind = descrValue[1];
    uint64_t itemsize;

    try {
        itemsize = std::stoull(descrValue.substr(2));
    } catch (const std::exception &e) {
        throw std::runtime_error(std::string("invalid size in dtype: ") + e.what());
    }

    return DType(kind, itemsize, byteorder);
}

bool checkFortranOrder(const std::string &dict) {
    size_t keyPos = dict.find("'fortran_order':");
    if (keyPos == std::string::npos)
        return false;

    size_t valuePos = dict.find("False", keyPos);
    if (valuePos == std::string::npos || valuePos != keyPos + 17)
        return false;

    return true;
}

void writeHeader(std::ostream &out, const std::vector<int64_t> &shape, const std::string &dtype) {
    std::stringstream headerDict;
    headerDict << "{";
    headerDict << "'descr': '" << dtype << "',";
    headerDict << "'fortran_order': False,";
    headerDict << "'shape': " << shapeToStr(shape);
    headerDict << "}";

    // calculate padding len
    std::string headerStr = headerDict.str();
    size_t paddingLen = 16 - ((10 + headerStr.size() + 1) % 16);
    headerStr += std::string(paddingLen, ' ') + '\n';

    // write magic string
    out.write(numpyMagicBytes.data(), numpyMagicBytes.size());

    // write version and HEADER_LEN
    size_t headerLen = headerStr.size();
    bool useVersion2 = headerLen > 65535;

    if (useVersion2) {
        out.put(static_cast<char>(0x02));
        out.put(static_cast<char>(0x00));
        out.put(static_cast<char>(headerLen & 0xFF));
        out.put(static_cast<char>((headerLen >> 8) & 0xFF));
        out.put(static_cast<char>((headerLen >> 16) & 0xFF));
        out.put(static_cast<char>((headerLen >> 24) & 0xFF));
    } else {
        out.put(static_cast<char>(0x01));
        out.put(static_cast<char>(0x00));
        out.put(static_cast<char>(headerLen & 0xFF));
        out.put(static_cast<char>((headerLen >> 8) & 0xFF));
    }

    // write header dict
    out.write(headerStr.c_str(), std::streamsize(headerStr.size()));
}

} // namespace

DType::DType(char kind, uint64_t itemsize) : byteorder(getEndianChar(itemsize)), kind(kind), itemsize(itemsize) {}

uint64_t DataPtr::size() const { return sizeOf(shape, dtype.itemsize); };

char numpyTypeEncoding(std::string_view numeric) {
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

DataPtr parse(const MemoryMap &mapped) {
    uint8_t majorVersion;
    uint32_t headerLen;
    size_t headerOffset = 0;

    // check magic string
    if (std::memcmp(mapped.ptr(), numpyMagicBytes.data(), numpyMagicBytes.size()) != 0)
        throw std::runtime_error("invalid NumPy file format");

    headerOffset += numpyMagicBytes.size();
    majorVersion = *reinterpret_cast<const uint8_t *>(mapped.ptr(headerOffset));
    headerOffset += 2;

    // check version
    if (majorVersion == 1) {
        uint16_t headerLenV1 = *reinterpret_cast<const uint16_t *>(mapped.ptr(headerOffset));
        headerLen = headerLenV1;
        headerOffset += sizeof(headerLenV1);

    } else if (majorVersion == 2) {
        headerLen = *reinterpret_cast<const uint32_t *>(mapped.ptr(headerOffset));
        headerOffset += sizeof(headerLen);

    } else {
        throw std::runtime_error("unsupported NumPy file version");
    }

    // parse header dict
    std::string dict(reinterpret_cast<const char *>(mapped.ptr(headerOffset)), headerLen);

    DataPtr dataPtr;
    // get dtype
    dataPtr.dtype = getDtype(dict);

    // check byte order
    char byteorder = dataPtr.dtype.byteorder;
    if ((isLittleEndian() && byteorder == '>') || (!isLittleEndian() && byteorder == '<'))
        throw std::runtime_error("mismatch in byte order");

    // check fortran order
    if (!checkFortranOrder(dict))
        throw std::runtime_error("only supported is fortran_order: False");

    // convert shape str to shape
    size_t shapeStart = dict.find('(');
    size_t shapeEnd = dict.find(')', shapeStart);
    std::string shapeStr = dict.substr(shapeStart + 1, shapeEnd - shapeStart - 1);
    dataPtr.shape = strToShape(shapeStr);

    // set dataPtr to start of numpy data
    headerOffset += headerLen;
    dataPtr.ptr = reinterpret_cast<const char *>(mapped.ptr(headerOffset));

    // consistency check: verify that all data is mapped
    if (headerOffset + dataPtr.size() > mapped.size())
        throw std::runtime_error("data size exceeds the mapped memory size");

    return dataPtr;
}

void write(const std::string &filename, const DataPtr &dataPtr) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("cannot open " + filename);
    }

    // write npy header to file
    writeHeader(file, dataPtr.shape, dtypeToStr(dataPtr.dtype));

    // write data to file
    file.write(reinterpret_cast<const char *>(dataPtr.ptr), std::streamsize(dataPtr.size()));
    file.close();
}

void write(const std::string &filename, const std::vector<int64_t> &shape, const DType &dtype,
           std::function<uint64_t(std::ostream &)> &&callback) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("cannot open " + filename);
    }

    // write npy header to file
    writeHeader(file, shape, dtypeToStr(dtype));

    // write data to file
    uint64_t size = callback(file);
    if (sizeOf(shape, dtype.itemsize) != size) {
        throw std::runtime_error("written wrong amount of data");
    }
    file.close();
}

void write(const std::string &filename, const char *ptr, const std::vector<int64_t> &shape, const char kind,
           const uint64_t &itemsize) {

    DType dtype{kind, itemsize};

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("cannot open " + filename);
    }

    // write npy header to file
    writeHeader(file, shape, dtypeToStr(dtype));

    // write data to file
    file.write(reinterpret_cast<const char *>(ptr), std::streamsize(sizeOf(shape, itemsize)));
    file.close();
}

} // namespace mlsdk::vgfutils::numpy
