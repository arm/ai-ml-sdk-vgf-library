/*
 * SPDX-FileCopyrightText: Copyright 2024-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "vgf/types.hpp"

namespace mlsdk::vgflib {

using EncodedDescriptorType = uint32_t;
static_assert(sizeof(EncodedDescriptorType) >= sizeof(DescriptorType));

constexpr EncodedDescriptorType NullOptDescriptorType() { return 0xFFFFFFFF; }

constexpr bool isSupportedScaledTensorValueFormat(FormatType format) {
    switch (format) {
    case 1000460002: // VK_FORMAT_R8_SFLOAT_FPENCODING_FLOAT8E4M3_ARM
    case 1000460003: // VK_FORMAT_R8_SFLOAT_FPENCODING_FLOAT8E5M2_ARM
    case 1000729000: // VK_FORMAT_OCPMXFP_32_SFLOAT_BLOCK_FPENCODING_FLOAT6E2M3_ARM
    case 1000729001: // VK_FORMAT_OCPMXFP_32_SFLOAT_BLOCK_FPENCODING_FLOAT6E3M2_ARM
    case 1000729002: // VK_FORMAT_OCPMXFP_32_SFLOAT_BLOCK_FPENCODING_FLOAT4E2M1_ARM
    case 1000729004: // VK_FORMAT_R8_SFLOAT_FPENCODING_MXINT8_ARM
        return true;
    default:
        return false;
    }
}

} // namespace mlsdk::vgflib
