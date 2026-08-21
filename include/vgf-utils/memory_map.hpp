/*
 * SPDX-FileCopyrightText: Copyright 2025-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <cstddef>
#include <string>

class MemoryMap {
  public:
    explicit MemoryMap(const std::string &filename);
    /// Maps filename and throws if its base address does not satisfy requiredAlignment.
    /// No aligned copy is made.
    MemoryMap(const std::string &filename, size_t requiredAlignment);
    MemoryMap(const MemoryMap &) = delete;
    MemoryMap &operator=(const MemoryMap &) = delete;
    MemoryMap(MemoryMap &&other) noexcept;
    MemoryMap &operator=(MemoryMap &&other) noexcept;

    ~MemoryMap();

    const void *ptr(size_t offset = 0) const;
    size_t size() const { return size_; }

  private:
#ifdef _WIN32
    void *hFile_{};
    void *hMap_{};
#else
    int fd_{-1};
#endif
    void *addr_{};
    size_t size_{};

    void reset() noexcept;
};
