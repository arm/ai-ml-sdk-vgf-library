/*
 * SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "section_index_table.hpp"

#include <gtest/gtest.h>

using namespace mlsdk::vgflib;

// Check that when sections are added, the correct size and alignment values are stored. Offset and padding are
// initially set to 0. Calling Update() recalculates the indices, updating offset and padding values. This test verifies
// that the offsets and padding calculated match the alignment requirements.
TEST(SectionIndexTable, Construction) {
    SectionIndexTable table;

    const auto &index0 = table.AddSection(16, 1);
    ASSERT_TRUE(index0.GetSize() == 16);
    ASSERT_TRUE(index0.GetAlignment() == 1);
    ASSERT_TRUE(index0.GetOffset() == 0);
    ASSERT_TRUE(index0.GetPadding() == 0);
    ASSERT_TRUE(index0.IsAligned() == true);

    const auto &index1 = table.AddSection(13, 8);
    ASSERT_TRUE(index1.GetSize() == 13);
    ASSERT_TRUE(index1.GetAlignment() == 8);
    ASSERT_TRUE(index1.GetOffset() == 0);
    ASSERT_TRUE(index1.GetPadding() == 0);
    ASSERT_TRUE(index1.IsAligned() == true);

    const auto &index2 = table.AddSection(5, 8);
    ASSERT_TRUE(index2.GetSize() == 5);
    ASSERT_TRUE(index2.GetAlignment() == 8);
    ASSERT_TRUE(index2.GetOffset() == 0);
    ASSERT_TRUE(index2.GetPadding() == 0);
    ASSERT_TRUE(index2.IsAligned() == true);

    const auto &index3 = table.AddSection(5, 8);
    ASSERT_TRUE(index3.GetSize() == 5);
    ASSERT_TRUE(index3.GetAlignment() == 8);
    ASSERT_TRUE(index3.GetOffset() == 0);
    ASSERT_TRUE(index3.GetPadding() == 0);
    ASSERT_TRUE(index3.IsAligned() == true);

    table.Update(); // Update all the indexes (offsets and padding)

    ASSERT_TRUE(index0.GetOffset() == 0);
    ASSERT_TRUE(index0.GetPadding() == 0);
    ASSERT_TRUE(index0.IsAligned() == true);
    ASSERT_TRUE(index0.EndOfData() == 16);
    ASSERT_TRUE(index0.NextOffset() == 16);

    ASSERT_TRUE(index1.GetOffset() == 16);
    ASSERT_TRUE(index1.GetPadding() == 3);
    ASSERT_TRUE(index1.IsAligned() == true);
    ASSERT_TRUE(index1.EndOfData() == 29);
    ASSERT_TRUE(index1.NextOffset() == 32);

    ASSERT_TRUE(index2.GetOffset() == 32);
    ASSERT_TRUE(index2.GetPadding() == 3);
    ASSERT_TRUE(index2.EndOfData() == 37);
    ASSERT_TRUE(index2.NextOffset() == 40);
    ASSERT_TRUE(index2.IsAligned() == true);

    ASSERT_TRUE(index3.GetOffset() == 40);
    ASSERT_TRUE(index3.GetPadding() == 0);
    ASSERT_TRUE(index3.EndOfData() == 45);
    ASSERT_TRUE(index3.NextOffset() == 45);
    ASSERT_TRUE(index3.IsAligned() == true);
}
