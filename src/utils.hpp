/*
 * SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>

namespace mlsdk::vgflib {

struct ByteRange {
    uint64_t offset{};
    uint64_t size{};
};

using PaddedByteRange = std::pair<ByteRange, uint64_t /* padding */>;

struct FixedRecordTableLayout {
    ByteRange records{};
    ByteRange payload{};
};

constexpr std::optional<uint64_t> checkedAdd(uint64_t lhs, uint64_t rhs) {
    if (lhs > std::numeric_limits<uint64_t>::max() - rhs) {
        return std::nullopt;
    }
    return lhs + rhs;
}

constexpr std::optional<uint64_t> checkedMul(uint64_t lhs, uint64_t rhs) {
    if (lhs != 0 && rhs > std::numeric_limits<uint64_t>::max() / lhs) {
        return std::nullopt;
    }
    return lhs * rhs;
}

constexpr std::optional<uint64_t> checkedAlignUp(uint64_t size, uint64_t alignment) {
    if (alignment == 0) {
        return std::nullopt;
    }

    const auto rounded = checkedAdd(size, alignment - 1);
    if (!rounded.has_value()) {
        return std::nullopt;
    }
    return (*rounded / alignment) * alignment;
}

constexpr bool byteRangeWithinBounds(ByteRange range, uint64_t payloadSize) {
    if (range.offset > payloadSize) {
        return false;
    }
    return range.size <= payloadSize - range.offset;
}

constexpr bool byteRangeCanBeAddressed(ByteRange range) {
#if SIZE_MAX < UINT64_MAX
    constexpr uint64_t sizeMax = static_cast<uint64_t>(std::numeric_limits<size_t>::max());
    if (range.offset > sizeMax || range.size > sizeMax) {
        return false;
    }
    return range.size <= sizeMax - range.offset;
#else
    (void)range;
    return true;
#endif
}

// Split a raw section into a fixed-size record table followed by a payload area.
// Used by raw section decoders to validate that the declared record count fits
// in the section and to compute the byte ranges for metadata records and
// variable-size payload data.
constexpr std::optional<FixedRecordTableLayout> splitFixedRecordTable(uint64_t sectionSize, uint64_t recordsOffset,
                                                                      uint64_t recordSize, uint64_t recordCount) {
    if (recordSize == 0 || recordsOffset > sectionSize) {
        return std::nullopt;
    }

    const uint64_t maxRecords = (sectionSize - recordsOffset) / recordSize;
    if (recordCount > maxRecords) {
        return std::nullopt;
    }

    const auto recordsSize = checkedMul(recordCount, recordSize);
    const auto payloadOffset =
        recordsSize.has_value() ? checkedAdd(recordsOffset, *recordsSize) : std::optional<uint64_t>{};
    if (!payloadOffset.has_value() || *payloadOffset > sectionSize) {
        return std::nullopt;
    }

    FixedRecordTableLayout layout{{recordsOffset, *recordsSize}, {*payloadOffset, sectionSize - *payloadOffset}};
    if (!byteRangeCanBeAddressed(layout.records) || !byteRangeCanBeAddressed(layout.payload)) {
        return std::nullopt;
    }

    return layout;
}

// Reserve a payload entry whose start is the current payload end and whose
// storage is padded up to the requested alignment. Used by raw section encoders
// to record the unpadded byte range in metadata while advancing the payload size
// by the padded storage size.
constexpr std::optional<PaddedByteRange> appendAlignedByteRange(uint64_t size, uint64_t alignment,
                                                                uint64_t &payloadSize) {
    const auto paddedSize = checkedAlignUp(size, alignment);
    if (!paddedSize.has_value()) {
        return std::nullopt;
    }
    const auto payloadEnd = checkedAdd(payloadSize, *paddedSize);
    if (!payloadEnd.has_value() || !byteRangeCanBeAddressed({payloadSize, *paddedSize})) {
        return std::nullopt;
    }

    const ByteRange range{payloadSize, size};
    payloadSize = *payloadEnd;
    return std::pair{range, *paddedSize};
}

} // namespace mlsdk::vgflib
