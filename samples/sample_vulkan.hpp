/*
 * SPDX-FileCopyrightText: Copyright 2024-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

// Samples can build without Vulkan headers by asking the generated helper header
// to provide int32_t-backed stand-ins for the Vk* enum types it references.
#define VGFLIB_VK_HELPERS

namespace mlsdk::vgflib::samples {

inline constexpr uint16_t VK_HEADER_VERSION = 999;
inline constexpr int32_t VK_DESCRIPTOR_TYPE_TENSOR_ARM = 1000460000;
inline constexpr int32_t VK_FORMAT_R8_SINT = 14;
inline constexpr int32_t VK_FORMAT_R32_SINT = 99;

} // namespace mlsdk::vgflib::samples
