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
    _hFile = reinterpret_cast<void *>(hFile);

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(_hFile, &fileSize)) {
        throw std::runtime_error("Failed to get file size for " + filename);
    }
    _size = static_cast<size_t>(fileSize.QuadPart);

    HANDLE hMap = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (hMap == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to create file mapping for file " + filename);
    }
    _hMap = reinterpret_cast<void *>(hMap);

    _addr = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
    if (_addr == nullptr) {
        throw std::runtime_error("MapViewOfFile failed for file " + filename);
    }
#else
    _fd = open(filename.c_str(), O_RDONLY);
    if (_fd < 0) {
        throw std::runtime_error("Could not open file " + filename);
    }
    struct stat st = {};
    if (fstat(_fd, &st) == -1) {
        throw std::runtime_error("Could not read attributes of file " + filename);
    }
    _size = size_t(st.st_size);

    _addr = mmap(nullptr, _size, PROT_READ, MAP_PRIVATE, _fd, 0);
    if (_addr == MAP_FAILED) {
        throw std::runtime_error("Failed to memory map the file " + filename);
    }
#endif
}

MemoryMap::~MemoryMap() {
#ifdef _WIN32
    UnmapViewOfFile(_addr);
    CloseHandle(reinterpret_cast<HANDLE>(_hMap));
    CloseHandle(reinterpret_cast<HANDLE>(_hFile));
#else
    if (_fd > 0) {
        munmap(_addr, _size);
        close(_fd);
    }
#endif
}

const void *MemoryMap::ptr(const size_t offset) const {
    if (offset >= _size) {
        throw std::runtime_error("offset " + std::to_string(offset) + " exceeds the mapped size " +
                                 std::to_string(_size));
    }
    return reinterpret_cast<const void *>(static_cast<char *>(_addr) + offset);
}
