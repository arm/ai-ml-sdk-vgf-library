/*
 * SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <cstdint>

namespace mlsdk::vgfutils {

/** @brief A binding and resource reference at one position in a binding-slot array. */
struct BindingSlot {
    BindingSlot() = default;
    BindingSlot(uint32_t index, uint32_t binding, uint32_t mrtIndex)
        : mIndex(index), mBinding(binding), mMrtIndex(mrtIndex) {}

    uint32_t mIndex{0};
    uint32_t mBinding{0};
    uint32_t mMrtIndex{0};
};

} // namespace mlsdk::vgfutils
