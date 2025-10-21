/*
 * SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#define VK_HEADER_VERSION 999

enum VkDescriptorType {
    VK_DESCRIPTOR_TYPE_TENSOR_ARM = 1000460000,
};

enum VkFormat {
    VK_FORMAT_R8_SINT = 14,
    VK_FORMAT_R32_SINT = 99,
};
