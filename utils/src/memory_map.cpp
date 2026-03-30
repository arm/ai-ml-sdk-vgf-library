/*
 * SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "vgf-utils/memory_map.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#    include <windows.h>

#    include <fileapi.h>
#else
#    include <sys/mman.h>
#    include <unistd.h>
#endif

#include <stdexcept>

MemoryMap::MemoryMap(const std::string &filename) {
#ifdef _WIN32
    HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Could not open file " + filename);
    }
    hFile_ = reinterpret_cast<void *>(hFile);

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile_, &fileSize)) {
        throw std::runtime_error("Failed to get file size for " + filename);
    }
    size_ = static_cast<size_t>(fileSize.QuadPart);

    HANDLE hMap = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (hMap == INVALID_HANDLE_VALUE) {
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
    size_ = size_t(st.st_size);

    addr_ = mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd_, 0);
    if (addr_ == MAP_FAILED) {
        throw std::runtime_error("Failed to memory map the file " + filename);
    }
#endif
}

MemoryMap::~MemoryMap() {
#ifdef _WIN32
    UnmapViewOfFile(addr_);
    CloseHandle(reinterpret_cast<HANDLE>(hMap_));
    CloseHandle(reinterpret_cast<HANDLE>(hFile_));
#else
    if (fd_ > 0) {
        munmap(addr_, size_);
        close(fd_);
    }
#endif
}

const void *MemoryMap::ptr(const size_t offset) const {
    if (offset >= size_) {
        throw std::runtime_error("offset " + std::to_string(offset) + " exceeds the mapped size " +
                                 std::to_string(size_));
    }
    return reinterpret_cast<const void *>(static_cast<char *>(addr_) + offset);
}
