/*
 * SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "vgf/types.hpp"

namespace mlsdk::vgflib {

using EncodedDescriptorType = uint32_t;
static_assert(sizeof(EncodedDescriptorType) >= sizeof(DescriptorType));

constexpr EncodedDescriptorType NullOptDescriptorType() { return 0xFFFFFFFF; }

} // namespace mlsdk::vgflib
