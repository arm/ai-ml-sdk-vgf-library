/*
 * SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <limits>
#include <optional>

namespace mlsdk::vgflib {

struct ByteRange {
    uint64_t offset{};
    uint64_t size{};
};

inline std::optional<uint64_t> checkedAdd(uint64_t lhs, uint64_t rhs) {
    if (lhs > std::numeric_limits<uint64_t>::max() - rhs) {
        return std::nullopt;
    }
    return lhs + rhs;
}

inline std::optional<uint64_t> checkedMul(uint64_t lhs, uint64_t rhs) {
    if (lhs != 0 && rhs > std::numeric_limits<uint64_t>::max() / lhs) {
        return std::nullopt;
    }
    return lhs * rhs;
}

inline std::optional<uint64_t> checkedAlignUp(uint64_t size, uint64_t alignment) {
    if (alignment == 0) {
        return std::nullopt;
    }

    const auto rounded = checkedAdd(size, alignment - 1);
    if (!rounded.has_value()) {
        return std::nullopt;
    }
    return (*rounded / alignment) * alignment;
}

inline bool byteRangeWithinBounds(ByteRange range, uint64_t payloadSize) {
    if (range.offset > payloadSize) {
        return false;
    }
    return range.size <= payloadSize - range.offset;
}

} // namespace mlsdk::vgflib
