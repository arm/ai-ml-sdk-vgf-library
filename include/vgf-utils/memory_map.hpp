/*
 * SPDX-FileCopyrightText: Copyright 2025-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <string>

class MemoryMap {
  public:
    explicit MemoryMap(const std::string &filename);
    MemoryMap(const MemoryMap &) = delete;
    MemoryMap &operator=(const MemoryMap &) = delete;
    MemoryMap(const MemoryMap &&) = delete;
    MemoryMap &operator=(const MemoryMap &&) = delete;

    ~MemoryMap();

    const void *ptr(size_t offset = 0) const;
    size_t size() const { return size_; }

  private:
#ifdef _WIN32
    void *hFile_;
    void *hMap_;
#else
    int fd_;
#endif
    void *addr_;
    size_t size_;
};
