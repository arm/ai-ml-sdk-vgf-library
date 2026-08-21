/*
 * SPDX-FileCopyrightText: Copyright 2025-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "vgf-utils/memory_map.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#    ifndef NOMINMAX
#        define NOMINMAX
#    endif
#    include <windows.h>

#    include <fileapi.h>
#else
#    include <sys/mman.h>
#    include <unistd.h>
#endif

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>

namespace {

bool isPowerOfTwo(size_t value) noexcept { return value != 0 && (value & (value - 1)) == 0; }

} // namespace

MemoryMap::MemoryMap(const std::string &filename) : MemoryMap(filename, 1) {}

MemoryMap::MemoryMap(const std::string &filename, size_t requiredAlignment) {
    if (!isPowerOfTwo(requiredAlignment)) {
        throw std::invalid_argument("MemoryMap alignment must be a non-zero power of two");
    }

    try {
#ifdef _WIN32
        const HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("Could not open file " + filename);
        }
        hFile_ = reinterpret_cast<void *>(hFile);

        LARGE_INTEGER fileSize;
        if (!GetFileSizeEx(hFile, &fileSize)) {
            throw std::runtime_error("Failed to get file size for " + filename);
        }
        if (fileSize.QuadPart < 0 ||
            static_cast<std::uintmax_t>(fileSize.QuadPart) > std::numeric_limits<size_t>::max()) {
            throw std::runtime_error("File is too large to memory map " + filename);
        }
        size_ = static_cast<size_t>(fileSize.QuadPart);

        const HANDLE hMap = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
        if (hMap == nullptr) {
            throw std::runtime_error("Failed to create file mapping for file " + filename);
        }
        hMap_ = reinterpret_cast<void *>(hMap);

        addr_ = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
        if (addr_ == nullptr) {
            throw std::runtime_error("MapViewOfFile failed for file " + filename);
        }
#else
        fd_ = open(filename.c_str(), O_RDONLY);
        if (fd_ < 0) {
            throw std::runtime_error("Could not open file " + filename);
        }
        struct stat st = {};
        if (fstat(fd_, &st) == -1) {
            throw std::runtime_error("Could not read attributes of file " + filename);
        }
        if (st.st_size < 0 || static_cast<std::uintmax_t>(st.st_size) > std::numeric_limits<size_t>::max()) {
            throw std::runtime_error("File is too large to memory map " + filename);
        }
        size_ = static_cast<size_t>(st.st_size);

        void *address = mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd_, 0);
        if (address == MAP_FAILED) {
            throw std::runtime_error("Failed to memory map the file " + filename);
        }
        addr_ = address;
#endif

        if ((reinterpret_cast<std::uintptr_t>(addr_) & (requiredAlignment - 1)) != 0) {
            throw std::runtime_error("Memory mapping for file " + filename +
                                     " does not satisfy the required alignment of " +
                                     std::to_string(requiredAlignment) + " bytes");
        }
    } catch (...) {
        reset();
        throw;
    }
}

MemoryMap::MemoryMap(MemoryMap &&other) noexcept
    :
#ifdef _WIN32
      hFile_(std::exchange(other.hFile_, nullptr)), hMap_(std::exchange(other.hMap_, nullptr)),
#else
      fd_(std::exchange(other.fd_, -1)),
#endif
      addr_(std::exchange(other.addr_, nullptr)), size_(std::exchange(other.size_, 0)) {
}

MemoryMap &MemoryMap::operator=(MemoryMap &&other) noexcept {
    if (this != &other) {
        reset();
#ifdef _WIN32
        hFile_ = std::exchange(other.hFile_, nullptr);
        hMap_ = std::exchange(other.hMap_, nullptr);
#else
        fd_ = std::exchange(other.fd_, -1);
#endif
        addr_ = std::exchange(other.addr_, nullptr);
        size_ = std::exchange(other.size_, 0);
    }
    return *this;
}

MemoryMap::~MemoryMap() { reset(); }

void MemoryMap::reset() noexcept {
#ifdef _WIN32
    if (addr_ != nullptr) {
        UnmapViewOfFile(addr_);
        addr_ = nullptr;
    }
    if (hMap_ != nullptr) {
        CloseHandle(reinterpret_cast<HANDLE>(hMap_));
        hMap_ = nullptr;
    }
    if (hFile_ != nullptr) {
        CloseHandle(reinterpret_cast<HANDLE>(hFile_));
        hFile_ = nullptr;
    }
#else
    if (addr_ != nullptr) {
        munmap(addr_, size_);
        addr_ = nullptr;
    }
    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
#endif
    size_ = 0;
}

const void *MemoryMap::ptr(const size_t offset) const {
    if (offset >= size_) {
        throw std::runtime_error("offset " + std::to_string(offset) + " exceeds the mapped size " +
                                 std::to_string(size_));
    }
    return reinterpret_cast<const void *>(static_cast<char *>(addr_) + offset);
}
