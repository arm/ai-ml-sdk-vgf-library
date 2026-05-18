/*
 * SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "utils.hpp"

#include <gtest/gtest.h>
#include <limits>

using namespace mlsdk::vgflib;

namespace {

constexpr bool byteRangeWithinBoundsConstexprTest() {
    return byteRangeWithinBounds({0, 0}, 0) && byteRangeWithinBounds({0, 4}, 8) && byteRangeWithinBounds({4, 4}, 8) &&
           byteRangeWithinBounds({8, 0}, 8) && !byteRangeWithinBounds({9, 0}, 8) && !byteRangeWithinBounds({5, 4}, 8);
}

constexpr bool byteRangeCanBeAddressedConstexprTest() {
    if (!byteRangeCanBeAddressed({0, 0}) || !byteRangeCanBeAddressed({4, 8})) {
        return false;
    }

#if SIZE_MAX < UINT64_MAX
    constexpr uint64_t sizeMax = static_cast<uint64_t>(std::numeric_limits<size_t>::max());
    return !byteRangeCanBeAddressed({sizeMax + 1, 0}) && !byteRangeCanBeAddressed({0, sizeMax + 1}) &&
           !byteRangeCanBeAddressed({sizeMax, 1});
#else
    return byteRangeCanBeAddressed({std::numeric_limits<uint64_t>::max(), 0});
#endif
}

constexpr bool splitFixedRecordTableConstexprTest() {
    const auto layout = splitFixedRecordTable(64, 16, 8, 3);
    if (!layout.has_value() || layout->records.offset != 16 || layout->records.size != 24 ||
        layout->payload.offset != 40 || layout->payload.size != 24) {
        return false;
    }

    const auto emptyPayloadLayout = splitFixedRecordTable(40, 16, 8, 3);
    return emptyPayloadLayout.has_value() && emptyPayloadLayout->records.offset == 16 &&
           emptyPayloadLayout->records.size == 24 && emptyPayloadLayout->payload.offset == 40 &&
           emptyPayloadLayout->payload.size == 0;
}

constexpr bool splitFixedRecordTableRejectsInvalidLayoutConstexprTest() {
    return !splitFixedRecordTable(16, 17, 8, 0).has_value() && !splitFixedRecordTable(16, 0, 0, 0).has_value() &&
           !splitFixedRecordTable(31, 16, 8, 2).has_value();
}

constexpr bool appendAlignedByteRangeConstexprTest() {
    uint64_t payloadSize = 0;
    const auto first = appendAlignedByteRange(5, 8, payloadSize);
    if (!first.has_value() || first->first.offset != 0 || first->first.size != 5 || first->second != 8 ||
        payloadSize != 8) {
        return false;
    }

    const auto second = appendAlignedByteRange(0, 8, payloadSize);
    return second.has_value() && second->first.offset == 8 && second->first.size == 0 && second->second == 0 &&
           payloadSize == 8;
}

static_assert(checkedAdd(1, 2).has_value() && *checkedAdd(1, 2) == 3);
static_assert(!checkedAdd(std::numeric_limits<uint64_t>::max(), 1).has_value());
static_assert(checkedMul(3, 4).has_value() && *checkedMul(3, 4) == 12);
static_assert(!checkedMul(std::numeric_limits<uint64_t>::max(), 2).has_value());
static_assert(checkedAlignUp(5, 8).has_value() && *checkedAlignUp(5, 8) == 8);
static_assert(!checkedAlignUp(5, 0).has_value());
static_assert(byteRangeWithinBoundsConstexprTest());
static_assert(byteRangeCanBeAddressedConstexprTest());
static_assert(splitFixedRecordTableConstexprTest());
static_assert(splitFixedRecordTableRejectsInvalidLayoutConstexprTest());
static_assert(appendAlignedByteRangeConstexprTest());

} // namespace

TEST(BinarySection, AppendAlignedByteRange) {
    uint64_t payloadSize = 0;

    const auto first = appendAlignedByteRange(5, 8, payloadSize);
    ASSERT_TRUE(first.has_value());
    const auto &[firstRange, firstPaddedSize] = *first;
    EXPECT_EQ(firstRange.offset, 0);
    EXPECT_EQ(firstRange.size, 5);
    EXPECT_EQ(firstPaddedSize, 8);
    EXPECT_EQ(payloadSize, 8);

    const auto second = appendAlignedByteRange(8, 8, payloadSize);
    ASSERT_TRUE(second.has_value());
    const auto &[secondRange, secondPaddedSize] = *second;
    EXPECT_EQ(secondRange.offset, 8);
    EXPECT_EQ(secondRange.size, 8);
    EXPECT_EQ(secondPaddedSize, 8);
    EXPECT_EQ(payloadSize, 16);

    const auto third = appendAlignedByteRange(0, 8, payloadSize);
    ASSERT_TRUE(third.has_value());
    const auto &[thirdRange, thirdPaddedSize] = *third;
    EXPECT_EQ(thirdRange.offset, 16);
    EXPECT_EQ(thirdRange.size, 0);
    EXPECT_EQ(thirdPaddedSize, 0);
    EXPECT_EQ(payloadSize, 16);
}

TEST(BinarySection, AppendAlignedByteRangeRejectsInvalidLayout) {
    uint64_t payloadSize = 0;
    EXPECT_FALSE(appendAlignedByteRange(1, 0, payloadSize).has_value());
    EXPECT_EQ(payloadSize, 0);

    payloadSize = std::numeric_limits<uint64_t>::max();
    EXPECT_FALSE(appendAlignedByteRange(1, 8, payloadSize).has_value());
    EXPECT_EQ(payloadSize, std::numeric_limits<uint64_t>::max());
}
