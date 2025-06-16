/*
 * SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

/* =======================================================================
    This file is generated based on the vulkan headers.
    !!! DO NOT MODIFY DIRECTLY !!!
    Use scripts/generate_helpers.py to update with a newer vulkan_core.h
   =======================================================================
*/
#pragma once

#if !defined(VK_HEADER_VERSION)
// Some tools, such as those that manipulate VGF files only, may not wish to
// depend on vulkan_core.h directly. For these cases define the below symbol
// before including this header.
#    if !defined(VGFLIB_VK_HELPERS)
#        error "Vulkan headers must be included before this file"
#    endif
#endif

#include "types.hpp"
#include <sstream>

namespace mlsdk::vgflib {

// Record the VK_HEADER_VERSION from the vulkan_core.h used to generate this file
const int32_t HEADER_VERSION_USED_FOR_HELPER_GENERATION = 305;

#if defined(VGFLIB_VK_HELPERS)
using VkDescriptorType = int32_t;
#endif

// Cast a VkDescriptorType type to a DescriptorType type
inline DescriptorType ToDescriptorType(VkDescriptorType e) { return static_cast<DescriptorType>(e); }

// Cast a DescriptorType type to a VkDescriptorType type
inline VkDescriptorType ToVkDescriptorType(DescriptorType e) { return static_cast<VkDescriptorType>(e); }

// Ensure validity of casted types
static_assert(sizeof(VkDescriptorType) == sizeof(DescriptorType));

// Convert a DescriptorType enum value to cstring name
inline std::string DescriptorTypeToName(DescriptorType e) {
    switch (static_cast<int>(e)) {
    case 0:
        return "VK_DESCRIPTOR_TYPE_SAMPLER";
    case 1:
        return "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER";
    case 2:
        return "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE";
    case 3:
        return "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE";
    case 4:
        return "VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER";
    case 5:
        return "VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER";
    case 6:
        return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER";
    case 7:
        return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER";
    case 8:
        return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC";
    case 9:
        return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC";
    case 10:
        return "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT";
    case 1000138000:
        return "VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK / VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT";
    case 1000150000:
        return "VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR";
    case 1000165000:
        return "VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV";
    case 1000440000:
        return "VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM";
    case 1000440001:
        return "VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM";
    case 1000452000:
        return "VK_DESCRIPTOR_TYPE_WEIGHTS_ARM";
    case 1000460000:
        return "VK_DESCRIPTOR_TYPE_TENSOR_ARM";
    case 1000351000:
        return "VK_DESCRIPTOR_TYPE_MUTABLE_EXT / VK_DESCRIPTOR_TYPE_MUTABLE_VALVE";
    case 0x7FFFFFFF:
        return "VK_DESCRIPTOR_TYPE_MAX_ENUM";
    default: {
        std::stringstream ss;
        ss << "Unknown(" << e << ")";
        return ss.str();
    }
    }
}

// Convert a string name to DescriptorTypeenum value
inline DescriptorType NameToDescriptorType(const std::string &str) {
    if (str == "VK_DESCRIPTOR_TYPE_SAMPLER") {
        return 0;
    }
    if (str == "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER") {
        return 1;
    }
    if (str == "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE") {
        return 2;
    }
    if (str == "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE") {
        return 3;
    }
    if (str == "VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER") {
        return 4;
    }
    if (str == "VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER") {
        return 5;
    }
    if (str == "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER") {
        return 6;
    }
    if (str == "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER") {
        return 7;
    }
    if (str == "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC") {
        return 8;
    }
    if (str == "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC") {
        return 9;
    }
    if (str == "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT") {
        return 10;
    }
    if (str == "VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK") {
        return 1000138000;
    }
    if (str == "VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR") {
        return 1000150000;
    }
    if (str == "VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV") {
        return 1000165000;
    }
    if (str == "VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM") {
        return 1000440000;
    }
    if (str == "VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM") {
        return 1000440001;
    }
    if (str == "VK_DESCRIPTOR_TYPE_WEIGHTS_ARM") {
        return 1000452000;
    }
    if (str == "VK_DESCRIPTOR_TYPE_TENSOR_ARM") {
        return 1000460000;
    }
    if (str == "VK_DESCRIPTOR_TYPE_MUTABLE_EXT") {
        return 1000351000;
    }
    if (str == "VK_DESCRIPTOR_TYPE_MAX_ENUM") {
        return 0x7FFFFFFF;
    }
    // Unknown name
    return -1;
}

// Validate that the decoded enum is valid given the Vulkan headers version in use. If this call
// fails, it indicates that either a newer vulkan_core.h was used by the VGF generator OR that the
// VGF is corrupted in some other way. Regenerating this header file with a newer Vulkan headers
// version may resolve the issue.
inline bool ValidateDecodedDescriptorType(DescriptorType e) {
    switch (static_cast<int>(e)) {
    case 0:          // VK_DESCRIPTOR_TYPE_SAMPLER
    case 1:          // VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
    case 2:          // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
    case 3:          // VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
    case 4:          // VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
    case 5:          // VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
    case 6:          // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    case 7:          // VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    case 8:          // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
    case 9:          // VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
    case 10:         // VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
    case 1000138000: // VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK / VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT
    case 1000150000: // VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR
    case 1000165000: // VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV
    case 1000440000: // VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM
    case 1000440001: // VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM
    case 1000452000: // VK_DESCRIPTOR_TYPE_WEIGHTS_ARM
    case 1000460000: // VK_DESCRIPTOR_TYPE_TENSOR_ARM
    case 1000351000: // VK_DESCRIPTOR_TYPE_MUTABLE_EXT / VK_DESCRIPTOR_TYPE_MUTABLE_VALVE
    case 0x7FFFFFFF: // VK_DESCRIPTOR_TYPE_MAX_ENUM
        return true;
    default:
        return false;
    }
}

#if defined(VGFLIB_VK_HELPERS)
using VkFormat = int32_t;
#endif

// Cast a VkFormat type to a FormatType type
inline FormatType ToFormatType(VkFormat e) { return static_cast<FormatType>(e); }

// Cast a FormatType type to a VkFormat type
inline VkFormat ToVkFormat(FormatType e) { return static_cast<VkFormat>(e); }

#if defined(VULKAN_RAII_HPP)
// Cast a VkFormat type to a vk::Format type
inline vk::Format ToRaiiFormat(VkFormat e) { return static_cast<vk::Format>(e); }

// Cast a FormatType type to a vk::Format type
inline vk::Format ToRaiiFormat(FormatType e) { return static_cast<vk::Format>(e); }

// Cast a vk::Format type to a VkFormat type
inline VkFormat ToVkFormat(vk::Format e) { return static_cast<VkFormat>(e); }

// Cast a vk::Format type to a FormatType type
inline FormatType ToFormatType(vk::Format e) { return static_cast<FormatType>(e); }

#endif

// Ensure validity of casted types
static_assert(sizeof(VkFormat) == sizeof(FormatType));

// Convert a FormatType enum value to cstring name
inline std::string FormatTypeToName(FormatType e) {
    switch (static_cast<int>(e)) {
    case 0:
        return "VK_FORMAT_UNDEFINED";
    case 1:
        return "VK_FORMAT_R4G4_UNORM_PACK8";
    case 2:
        return "VK_FORMAT_R4G4B4A4_UNORM_PACK16";
    case 3:
        return "VK_FORMAT_B4G4R4A4_UNORM_PACK16";
    case 4:
        return "VK_FORMAT_R5G6B5_UNORM_PACK16";
    case 5:
        return "VK_FORMAT_B5G6R5_UNORM_PACK16";
    case 6:
        return "VK_FORMAT_R5G5B5A1_UNORM_PACK16";
    case 7:
        return "VK_FORMAT_B5G5R5A1_UNORM_PACK16";
    case 8:
        return "VK_FORMAT_A1R5G5B5_UNORM_PACK16";
    case 9:
        return "VK_FORMAT_R8_UNORM";
    case 10:
        return "VK_FORMAT_R8_SNORM";
    case 11:
        return "VK_FORMAT_R8_USCALED";
    case 12:
        return "VK_FORMAT_R8_SSCALED";
    case 13:
        return "VK_FORMAT_R8_UINT";
    case 14:
        return "VK_FORMAT_R8_SINT";
    case 15:
        return "VK_FORMAT_R8_SRGB";
    case 16:
        return "VK_FORMAT_R8G8_UNORM";
    case 17:
        return "VK_FORMAT_R8G8_SNORM";
    case 18:
        return "VK_FORMAT_R8G8_USCALED";
    case 19:
        return "VK_FORMAT_R8G8_SSCALED";
    case 20:
        return "VK_FORMAT_R8G8_UINT";
    case 21:
        return "VK_FORMAT_R8G8_SINT";
    case 22:
        return "VK_FORMAT_R8G8_SRGB";
    case 23:
        return "VK_FORMAT_R8G8B8_UNORM";
    case 24:
        return "VK_FORMAT_R8G8B8_SNORM";
    case 25:
        return "VK_FORMAT_R8G8B8_USCALED";
    case 26:
        return "VK_FORMAT_R8G8B8_SSCALED";
    case 27:
        return "VK_FORMAT_R8G8B8_UINT";
    case 28:
        return "VK_FORMAT_R8G8B8_SINT";
    case 29:
        return "VK_FORMAT_R8G8B8_SRGB";
    case 30:
        return "VK_FORMAT_B8G8R8_UNORM";
    case 31:
        return "VK_FORMAT_B8G8R8_SNORM";
    case 32:
        return "VK_FORMAT_B8G8R8_USCALED";
    case 33:
        return "VK_FORMAT_B8G8R8_SSCALED";
    case 34:
        return "VK_FORMAT_B8G8R8_UINT";
    case 35:
        return "VK_FORMAT_B8G8R8_SINT";
    case 36:
        return "VK_FORMAT_B8G8R8_SRGB";
    case 37:
        return "VK_FORMAT_R8G8B8A8_UNORM";
    case 38:
        return "VK_FORMAT_R8G8B8A8_SNORM";
    case 39:
        return "VK_FORMAT_R8G8B8A8_USCALED";
    case 40:
        return "VK_FORMAT_R8G8B8A8_SSCALED";
    case 41:
        return "VK_FORMAT_R8G8B8A8_UINT";
    case 42:
        return "VK_FORMAT_R8G8B8A8_SINT";
    case 43:
        return "VK_FORMAT_R8G8B8A8_SRGB";
    case 44:
        return "VK_FORMAT_B8G8R8A8_UNORM";
    case 45:
        return "VK_FORMAT_B8G8R8A8_SNORM";
    case 46:
        return "VK_FORMAT_B8G8R8A8_USCALED";
    case 47:
        return "VK_FORMAT_B8G8R8A8_SSCALED";
    case 48:
        return "VK_FORMAT_B8G8R8A8_UINT";
    case 49:
        return "VK_FORMAT_B8G8R8A8_SINT";
    case 50:
        return "VK_FORMAT_B8G8R8A8_SRGB";
    case 51:
        return "VK_FORMAT_A8B8G8R8_UNORM_PACK32";
    case 52:
        return "VK_FORMAT_A8B8G8R8_SNORM_PACK32";
    case 53:
        return "VK_FORMAT_A8B8G8R8_USCALED_PACK32";
    case 54:
        return "VK_FORMAT_A8B8G8R8_SSCALED_PACK32";
    case 55:
        return "VK_FORMAT_A8B8G8R8_UINT_PACK32";
    case 56:
        return "VK_FORMAT_A8B8G8R8_SINT_PACK32";
    case 57:
        return "VK_FORMAT_A8B8G8R8_SRGB_PACK32";
    case 58:
        return "VK_FORMAT_A2R10G10B10_UNORM_PACK32";
    case 59:
        return "VK_FORMAT_A2R10G10B10_SNORM_PACK32";
    case 60:
        return "VK_FORMAT_A2R10G10B10_USCALED_PACK32";
    case 61:
        return "VK_FORMAT_A2R10G10B10_SSCALED_PACK32";
    case 62:
        return "VK_FORMAT_A2R10G10B10_UINT_PACK32";
    case 63:
        return "VK_FORMAT_A2R10G10B10_SINT_PACK32";
    case 64:
        return "VK_FORMAT_A2B10G10R10_UNORM_PACK32";
    case 65:
        return "VK_FORMAT_A2B10G10R10_SNORM_PACK32";
    case 66:
        return "VK_FORMAT_A2B10G10R10_USCALED_PACK32";
    case 67:
        return "VK_FORMAT_A2B10G10R10_SSCALED_PACK32";
    case 68:
        return "VK_FORMAT_A2B10G10R10_UINT_PACK32";
    case 69:
        return "VK_FORMAT_A2B10G10R10_SINT_PACK32";
    case 70:
        return "VK_FORMAT_R16_UNORM";
    case 71:
        return "VK_FORMAT_R16_SNORM";
    case 72:
        return "VK_FORMAT_R16_USCALED";
    case 73:
        return "VK_FORMAT_R16_SSCALED";
    case 74:
        return "VK_FORMAT_R16_UINT";
    case 75:
        return "VK_FORMAT_R16_SINT";
    case 76:
        return "VK_FORMAT_R16_SFLOAT";
    case 77:
        return "VK_FORMAT_R16G16_UNORM";
    case 78:
        return "VK_FORMAT_R16G16_SNORM";
    case 79:
        return "VK_FORMAT_R16G16_USCALED";
    case 80:
        return "VK_FORMAT_R16G16_SSCALED";
    case 81:
        return "VK_FORMAT_R16G16_UINT";
    case 82:
        return "VK_FORMAT_R16G16_SINT";
    case 83:
        return "VK_FORMAT_R16G16_SFLOAT";
    case 84:
        return "VK_FORMAT_R16G16B16_UNORM";
    case 85:
        return "VK_FORMAT_R16G16B16_SNORM";
    case 86:
        return "VK_FORMAT_R16G16B16_USCALED";
    case 87:
        return "VK_FORMAT_R16G16B16_SSCALED";
    case 88:
        return "VK_FORMAT_R16G16B16_UINT";
    case 89:
        return "VK_FORMAT_R16G16B16_SINT";
    case 90:
        return "VK_FORMAT_R16G16B16_SFLOAT";
    case 91:
        return "VK_FORMAT_R16G16B16A16_UNORM";
    case 92:
        return "VK_FORMAT_R16G16B16A16_SNORM";
    case 93:
        return "VK_FORMAT_R16G16B16A16_USCALED";
    case 94:
        return "VK_FORMAT_R16G16B16A16_SSCALED";
    case 95:
        return "VK_FORMAT_R16G16B16A16_UINT";
    case 96:
        return "VK_FORMAT_R16G16B16A16_SINT";
    case 97:
        return "VK_FORMAT_R16G16B16A16_SFLOAT";
    case 98:
        return "VK_FORMAT_R32_UINT";
    case 99:
        return "VK_FORMAT_R32_SINT";
    case 100:
        return "VK_FORMAT_R32_SFLOAT";
    case 101:
        return "VK_FORMAT_R32G32_UINT";
    case 102:
        return "VK_FORMAT_R32G32_SINT";
    case 103:
        return "VK_FORMAT_R32G32_SFLOAT";
    case 104:
        return "VK_FORMAT_R32G32B32_UINT";
    case 105:
        return "VK_FORMAT_R32G32B32_SINT";
    case 106:
        return "VK_FORMAT_R32G32B32_SFLOAT";
    case 107:
        return "VK_FORMAT_R32G32B32A32_UINT";
    case 108:
        return "VK_FORMAT_R32G32B32A32_SINT";
    case 109:
        return "VK_FORMAT_R32G32B32A32_SFLOAT";
    case 110:
        return "VK_FORMAT_R64_UINT";
    case 111:
        return "VK_FORMAT_R64_SINT";
    case 112:
        return "VK_FORMAT_R64_SFLOAT";
    case 113:
        return "VK_FORMAT_R64G64_UINT";
    case 114:
        return "VK_FORMAT_R64G64_SINT";
    case 115:
        return "VK_FORMAT_R64G64_SFLOAT";
    case 116:
        return "VK_FORMAT_R64G64B64_UINT";
    case 117:
        return "VK_FORMAT_R64G64B64_SINT";
    case 118:
        return "VK_FORMAT_R64G64B64_SFLOAT";
    case 119:
        return "VK_FORMAT_R64G64B64A64_UINT";
    case 120:
        return "VK_FORMAT_R64G64B64A64_SINT";
    case 121:
        return "VK_FORMAT_R64G64B64A64_SFLOAT";
    case 122:
        return "VK_FORMAT_B10G11R11_UFLOAT_PACK32";
    case 123:
        return "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32";
    case 124:
        return "VK_FORMAT_D16_UNORM";
    case 125:
        return "VK_FORMAT_X8_D24_UNORM_PACK32";
    case 126:
        return "VK_FORMAT_D32_SFLOAT";
    case 127:
        return "VK_FORMAT_S8_UINT";
    case 128:
        return "VK_FORMAT_D16_UNORM_S8_UINT";
    case 129:
        return "VK_FORMAT_D24_UNORM_S8_UINT";
    case 130:
        return "VK_FORMAT_D32_SFLOAT_S8_UINT";
    case 131:
        return "VK_FORMAT_BC1_RGB_UNORM_BLOCK";
    case 132:
        return "VK_FORMAT_BC1_RGB_SRGB_BLOCK";
    case 133:
        return "VK_FORMAT_BC1_RGBA_UNORM_BLOCK";
    case 134:
        return "VK_FORMAT_BC1_RGBA_SRGB_BLOCK";
    case 135:
        return "VK_FORMAT_BC2_UNORM_BLOCK";
    case 136:
        return "VK_FORMAT_BC2_SRGB_BLOCK";
    case 137:
        return "VK_FORMAT_BC3_UNORM_BLOCK";
    case 138:
        return "VK_FORMAT_BC3_SRGB_BLOCK";
    case 139:
        return "VK_FORMAT_BC4_UNORM_BLOCK";
    case 140:
        return "VK_FORMAT_BC4_SNORM_BLOCK";
    case 141:
        return "VK_FORMAT_BC5_UNORM_BLOCK";
    case 142:
        return "VK_FORMAT_BC5_SNORM_BLOCK";
    case 143:
        return "VK_FORMAT_BC6H_UFLOAT_BLOCK";
    case 144:
        return "VK_FORMAT_BC6H_SFLOAT_BLOCK";
    case 145:
        return "VK_FORMAT_BC7_UNORM_BLOCK";
    case 146:
        return "VK_FORMAT_BC7_SRGB_BLOCK";
    case 147:
        return "VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK";
    case 148:
        return "VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK";
    case 149:
        return "VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK";
    case 150:
        return "VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK";
    case 151:
        return "VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK";
    case 152:
        return "VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK";
    case 153:
        return "VK_FORMAT_EAC_R11_UNORM_BLOCK";
    case 154:
        return "VK_FORMAT_EAC_R11_SNORM_BLOCK";
    case 155:
        return "VK_FORMAT_EAC_R11G11_UNORM_BLOCK";
    case 156:
        return "VK_FORMAT_EAC_R11G11_SNORM_BLOCK";
    case 157:
        return "VK_FORMAT_ASTC_4x4_UNORM_BLOCK";
    case 158:
        return "VK_FORMAT_ASTC_4x4_SRGB_BLOCK";
    case 159:
        return "VK_FORMAT_ASTC_5x4_UNORM_BLOCK";
    case 160:
        return "VK_FORMAT_ASTC_5x4_SRGB_BLOCK";
    case 161:
        return "VK_FORMAT_ASTC_5x5_UNORM_BLOCK";
    case 162:
        return "VK_FORMAT_ASTC_5x5_SRGB_BLOCK";
    case 163:
        return "VK_FORMAT_ASTC_6x5_UNORM_BLOCK";
    case 164:
        return "VK_FORMAT_ASTC_6x5_SRGB_BLOCK";
    case 165:
        return "VK_FORMAT_ASTC_6x6_UNORM_BLOCK";
    case 166:
        return "VK_FORMAT_ASTC_6x6_SRGB_BLOCK";
    case 167:
        return "VK_FORMAT_ASTC_8x5_UNORM_BLOCK";
    case 168:
        return "VK_FORMAT_ASTC_8x5_SRGB_BLOCK";
    case 169:
        return "VK_FORMAT_ASTC_8x6_UNORM_BLOCK";
    case 170:
        return "VK_FORMAT_ASTC_8x6_SRGB_BLOCK";
    case 171:
        return "VK_FORMAT_ASTC_8x8_UNORM_BLOCK";
    case 172:
        return "VK_FORMAT_ASTC_8x8_SRGB_BLOCK";
    case 173:
        return "VK_FORMAT_ASTC_10x5_UNORM_BLOCK";
    case 174:
        return "VK_FORMAT_ASTC_10x5_SRGB_BLOCK";
    case 175:
        return "VK_FORMAT_ASTC_10x6_UNORM_BLOCK";
    case 176:
        return "VK_FORMAT_ASTC_10x6_SRGB_BLOCK";
    case 177:
        return "VK_FORMAT_ASTC_10x8_UNORM_BLOCK";
    case 178:
        return "VK_FORMAT_ASTC_10x8_SRGB_BLOCK";
    case 179:
        return "VK_FORMAT_ASTC_10x10_UNORM_BLOCK";
    case 180:
        return "VK_FORMAT_ASTC_10x10_SRGB_BLOCK";
    case 181:
        return "VK_FORMAT_ASTC_12x10_UNORM_BLOCK";
    case 182:
        return "VK_FORMAT_ASTC_12x10_SRGB_BLOCK";
    case 183:
        return "VK_FORMAT_ASTC_12x12_UNORM_BLOCK";
    case 184:
        return "VK_FORMAT_ASTC_12x12_SRGB_BLOCK";
    case 1000156000:
        return "VK_FORMAT_G8B8G8R8_422_UNORM / VK_FORMAT_G8B8G8R8_422_UNORM_KHR";
    case 1000156001:
        return "VK_FORMAT_B8G8R8G8_422_UNORM / VK_FORMAT_B8G8R8G8_422_UNORM_KHR";
    case 1000156002:
        return "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM / VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR";
    case 1000156003:
        return "VK_FORMAT_G8_B8R8_2PLANE_420_UNORM / VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR";
    case 1000156004:
        return "VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM / VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR";
    case 1000156005:
        return "VK_FORMAT_G8_B8R8_2PLANE_422_UNORM / VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR";
    case 1000156006:
        return "VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM / VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR";
    case 1000156007:
        return "VK_FORMAT_R10X6_UNORM_PACK16 / VK_FORMAT_R10X6_UNORM_PACK16_KHR";
    case 1000156008:
        return "VK_FORMAT_R10X6G10X6_UNORM_2PACK16 / VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR";
    case 1000156009:
        return "VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16 / VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR";
    case 1000156010:
        return "VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16 / "
               "VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR";
    case 1000156011:
        return "VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16 / "
               "VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR";
    case 1000156012:
        return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 / "
               "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR";
    case 1000156013:
        return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16 / "
               "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR";
    case 1000156014:
        return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 / "
               "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR";
    case 1000156015:
        return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16 / "
               "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR";
    case 1000156016:
        return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 / "
               "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR";
    case 1000156017:
        return "VK_FORMAT_R12X4_UNORM_PACK16 / VK_FORMAT_R12X4_UNORM_PACK16_KHR";
    case 1000156018:
        return "VK_FORMAT_R12X4G12X4_UNORM_2PACK16 / VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR";
    case 1000156019:
        return "VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16 / VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR";
    case 1000156020:
        return "VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16 / "
               "VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR";
    case 1000156021:
        return "VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16 / "
               "VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR";
    case 1000156022:
        return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 / "
               "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR";
    case 1000156023:
        return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16 / "
               "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR";
    case 1000156024:
        return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 / "
               "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR";
    case 1000156025:
        return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16 / "
               "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR";
    case 1000156026:
        return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 / "
               "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR";
    case 1000156027:
        return "VK_FORMAT_G16B16G16R16_422_UNORM / VK_FORMAT_G16B16G16R16_422_UNORM_KHR";
    case 1000156028:
        return "VK_FORMAT_B16G16R16G16_422_UNORM / VK_FORMAT_B16G16R16G16_422_UNORM_KHR";
    case 1000156029:
        return "VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM / VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR";
    case 1000156030:
        return "VK_FORMAT_G16_B16R16_2PLANE_420_UNORM / VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR";
    case 1000156031:
        return "VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM / VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR";
    case 1000156032:
        return "VK_FORMAT_G16_B16R16_2PLANE_422_UNORM / VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR";
    case 1000156033:
        return "VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM / VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR";
    case 1000330000:
        return "VK_FORMAT_G8_B8R8_2PLANE_444_UNORM / VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT";
    case 1000330001:
        return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16 / "
               "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT";
    case 1000330002:
        return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16 / "
               "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT";
    case 1000330003:
        return "VK_FORMAT_G16_B16R16_2PLANE_444_UNORM / VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT";
    case 1000340000:
        return "VK_FORMAT_A4R4G4B4_UNORM_PACK16 / VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT";
    case 1000340001:
        return "VK_FORMAT_A4B4G4R4_UNORM_PACK16 / VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT";
    case 1000066000:
        return "VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK / VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT";
    case 1000066001:
        return "VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK / VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT";
    case 1000066002:
        return "VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK / VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT";
    case 1000066003:
        return "VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK / VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT";
    case 1000066004:
        return "VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK / VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT";
    case 1000066005:
        return "VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK / VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT";
    case 1000066006:
        return "VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK / VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT";
    case 1000066007:
        return "VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK / VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT";
    case 1000066008:
        return "VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK / VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT";
    case 1000066009:
        return "VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK / VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT";
    case 1000066010:
        return "VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK / VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT";
    case 1000066011:
        return "VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK / VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT";
    case 1000066012:
        return "VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK / VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT";
    case 1000066013:
        return "VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK / VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT";
    case 1000470000:
        return "VK_FORMAT_A1B5G5R5_UNORM_PACK16 / VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR";
    case 1000470001:
        return "VK_FORMAT_A8_UNORM / VK_FORMAT_A8_UNORM_KHR";
    case 1000054000:
        return "VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG";
    case 1000054001:
        return "VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG";
    case 1000054002:
        return "VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG";
    case 1000054003:
        return "VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG";
    case 1000054004:
        return "VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG";
    case 1000054005:
        return "VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG";
    case 1000054006:
        return "VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG";
    case 1000054007:
        return "VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG";
    case 1000452000:
        return "VK_FORMAT_SBS80_ARM";
    case 1000460000:
        return "VK_FORMAT_R8_BOOL_ARM";
    case 1000464000:
        return "VK_FORMAT_R16G16_SFIXED5_NV / VK_FORMAT_R16G16_S10_5_NV";
    case 0x7FFFFFFF:
        return "VK_FORMAT_MAX_ENUM";
    default: {
        std::stringstream ss;
        ss << "Unknown(" << e << ")";
        return ss.str();
    }
    }
}

// Convert a string name to FormatTypeenum value
inline FormatType NameToFormatType(const std::string &str) {
    if (str == "VK_FORMAT_UNDEFINED") {
        return 0;
    }
    if (str == "VK_FORMAT_R4G4_UNORM_PACK8") {
        return 1;
    }
    if (str == "VK_FORMAT_R4G4B4A4_UNORM_PACK16") {
        return 2;
    }
    if (str == "VK_FORMAT_B4G4R4A4_UNORM_PACK16") {
        return 3;
    }
    if (str == "VK_FORMAT_R5G6B5_UNORM_PACK16") {
        return 4;
    }
    if (str == "VK_FORMAT_B5G6R5_UNORM_PACK16") {
        return 5;
    }
    if (str == "VK_FORMAT_R5G5B5A1_UNORM_PACK16") {
        return 6;
    }
    if (str == "VK_FORMAT_B5G5R5A1_UNORM_PACK16") {
        return 7;
    }
    if (str == "VK_FORMAT_A1R5G5B5_UNORM_PACK16") {
        return 8;
    }
    if (str == "VK_FORMAT_R8_UNORM") {
        return 9;
    }
    if (str == "VK_FORMAT_R8_SNORM") {
        return 10;
    }
    if (str == "VK_FORMAT_R8_USCALED") {
        return 11;
    }
    if (str == "VK_FORMAT_R8_SSCALED") {
        return 12;
    }
    if (str == "VK_FORMAT_R8_UINT") {
        return 13;
    }
    if (str == "VK_FORMAT_R8_SINT") {
        return 14;
    }
    if (str == "VK_FORMAT_R8_SRGB") {
        return 15;
    }
    if (str == "VK_FORMAT_R8G8_UNORM") {
        return 16;
    }
    if (str == "VK_FORMAT_R8G8_SNORM") {
        return 17;
    }
    if (str == "VK_FORMAT_R8G8_USCALED") {
        return 18;
    }
    if (str == "VK_FORMAT_R8G8_SSCALED") {
        return 19;
    }
    if (str == "VK_FORMAT_R8G8_UINT") {
        return 20;
    }
    if (str == "VK_FORMAT_R8G8_SINT") {
        return 21;
    }
    if (str == "VK_FORMAT_R8G8_SRGB") {
        return 22;
    }
    if (str == "VK_FORMAT_R8G8B8_UNORM") {
        return 23;
    }
    if (str == "VK_FORMAT_R8G8B8_SNORM") {
        return 24;
    }
    if (str == "VK_FORMAT_R8G8B8_USCALED") {
        return 25;
    }
    if (str == "VK_FORMAT_R8G8B8_SSCALED") {
        return 26;
    }
    if (str == "VK_FORMAT_R8G8B8_UINT") {
        return 27;
    }
    if (str == "VK_FORMAT_R8G8B8_SINT") {
        return 28;
    }
    if (str == "VK_FORMAT_R8G8B8_SRGB") {
        return 29;
    }
    if (str == "VK_FORMAT_B8G8R8_UNORM") {
        return 30;
    }
    if (str == "VK_FORMAT_B8G8R8_SNORM") {
        return 31;
    }
    if (str == "VK_FORMAT_B8G8R8_USCALED") {
        return 32;
    }
    if (str == "VK_FORMAT_B8G8R8_SSCALED") {
        return 33;
    }
    if (str == "VK_FORMAT_B8G8R8_UINT") {
        return 34;
    }
    if (str == "VK_FORMAT_B8G8R8_SINT") {
        return 35;
    }
    if (str == "VK_FORMAT_B8G8R8_SRGB") {
        return 36;
    }
    if (str == "VK_FORMAT_R8G8B8A8_UNORM") {
        return 37;
    }
    if (str == "VK_FORMAT_R8G8B8A8_SNORM") {
        return 38;
    }
    if (str == "VK_FORMAT_R8G8B8A8_USCALED") {
        return 39;
    }
    if (str == "VK_FORMAT_R8G8B8A8_SSCALED") {
        return 40;
    }
    if (str == "VK_FORMAT_R8G8B8A8_UINT") {
        return 41;
    }
    if (str == "VK_FORMAT_R8G8B8A8_SINT") {
        return 42;
    }
    if (str == "VK_FORMAT_R8G8B8A8_SRGB") {
        return 43;
    }
    if (str == "VK_FORMAT_B8G8R8A8_UNORM") {
        return 44;
    }
    if (str == "VK_FORMAT_B8G8R8A8_SNORM") {
        return 45;
    }
    if (str == "VK_FORMAT_B8G8R8A8_USCALED") {
        return 46;
    }
    if (str == "VK_FORMAT_B8G8R8A8_SSCALED") {
        return 47;
    }
    if (str == "VK_FORMAT_B8G8R8A8_UINT") {
        return 48;
    }
    if (str == "VK_FORMAT_B8G8R8A8_SINT") {
        return 49;
    }
    if (str == "VK_FORMAT_B8G8R8A8_SRGB") {
        return 50;
    }
    if (str == "VK_FORMAT_A8B8G8R8_UNORM_PACK32") {
        return 51;
    }
    if (str == "VK_FORMAT_A8B8G8R8_SNORM_PACK32") {
        return 52;
    }
    if (str == "VK_FORMAT_A8B8G8R8_USCALED_PACK32") {
        return 53;
    }
    if (str == "VK_FORMAT_A8B8G8R8_SSCALED_PACK32") {
        return 54;
    }
    if (str == "VK_FORMAT_A8B8G8R8_UINT_PACK32") {
        return 55;
    }
    if (str == "VK_FORMAT_A8B8G8R8_SINT_PACK32") {
        return 56;
    }
    if (str == "VK_FORMAT_A8B8G8R8_SRGB_PACK32") {
        return 57;
    }
    if (str == "VK_FORMAT_A2R10G10B10_UNORM_PACK32") {
        return 58;
    }
    if (str == "VK_FORMAT_A2R10G10B10_SNORM_PACK32") {
        return 59;
    }
    if (str == "VK_FORMAT_A2R10G10B10_USCALED_PACK32") {
        return 60;
    }
    if (str == "VK_FORMAT_A2R10G10B10_SSCALED_PACK32") {
        return 61;
    }
    if (str == "VK_FORMAT_A2R10G10B10_UINT_PACK32") {
        return 62;
    }
    if (str == "VK_FORMAT_A2R10G10B10_SINT_PACK32") {
        return 63;
    }
    if (str == "VK_FORMAT_A2B10G10R10_UNORM_PACK32") {
        return 64;
    }
    if (str == "VK_FORMAT_A2B10G10R10_SNORM_PACK32") {
        return 65;
    }
    if (str == "VK_FORMAT_A2B10G10R10_USCALED_PACK32") {
        return 66;
    }
    if (str == "VK_FORMAT_A2B10G10R10_SSCALED_PACK32") {
        return 67;
    }
    if (str == "VK_FORMAT_A2B10G10R10_UINT_PACK32") {
        return 68;
    }
    if (str == "VK_FORMAT_A2B10G10R10_SINT_PACK32") {
        return 69;
    }
    if (str == "VK_FORMAT_R16_UNORM") {
        return 70;
    }
    if (str == "VK_FORMAT_R16_SNORM") {
        return 71;
    }
    if (str == "VK_FORMAT_R16_USCALED") {
        return 72;
    }
    if (str == "VK_FORMAT_R16_SSCALED") {
        return 73;
    }
    if (str == "VK_FORMAT_R16_UINT") {
        return 74;
    }
    if (str == "VK_FORMAT_R16_SINT") {
        return 75;
    }
    if (str == "VK_FORMAT_R16_SFLOAT") {
        return 76;
    }
    if (str == "VK_FORMAT_R16G16_UNORM") {
        return 77;
    }
    if (str == "VK_FORMAT_R16G16_SNORM") {
        return 78;
    }
    if (str == "VK_FORMAT_R16G16_USCALED") {
        return 79;
    }
    if (str == "VK_FORMAT_R16G16_SSCALED") {
        return 80;
    }
    if (str == "VK_FORMAT_R16G16_UINT") {
        return 81;
    }
    if (str == "VK_FORMAT_R16G16_SINT") {
        return 82;
    }
    if (str == "VK_FORMAT_R16G16_SFLOAT") {
        return 83;
    }
    if (str == "VK_FORMAT_R16G16B16_UNORM") {
        return 84;
    }
    if (str == "VK_FORMAT_R16G16B16_SNORM") {
        return 85;
    }
    if (str == "VK_FORMAT_R16G16B16_USCALED") {
        return 86;
    }
    if (str == "VK_FORMAT_R16G16B16_SSCALED") {
        return 87;
    }
    if (str == "VK_FORMAT_R16G16B16_UINT") {
        return 88;
    }
    if (str == "VK_FORMAT_R16G16B16_SINT") {
        return 89;
    }
    if (str == "VK_FORMAT_R16G16B16_SFLOAT") {
        return 90;
    }
    if (str == "VK_FORMAT_R16G16B16A16_UNORM") {
        return 91;
    }
    if (str == "VK_FORMAT_R16G16B16A16_SNORM") {
        return 92;
    }
    if (str == "VK_FORMAT_R16G16B16A16_USCALED") {
        return 93;
    }
    if (str == "VK_FORMAT_R16G16B16A16_SSCALED") {
        return 94;
    }
    if (str == "VK_FORMAT_R16G16B16A16_UINT") {
        return 95;
    }
    if (str == "VK_FORMAT_R16G16B16A16_SINT") {
        return 96;
    }
    if (str == "VK_FORMAT_R16G16B16A16_SFLOAT") {
        return 97;
    }
    if (str == "VK_FORMAT_R32_UINT") {
        return 98;
    }
    if (str == "VK_FORMAT_R32_SINT") {
        return 99;
    }
    if (str == "VK_FORMAT_R32_SFLOAT") {
        return 100;
    }
    if (str == "VK_FORMAT_R32G32_UINT") {
        return 101;
    }
    if (str == "VK_FORMAT_R32G32_SINT") {
        return 102;
    }
    if (str == "VK_FORMAT_R32G32_SFLOAT") {
        return 103;
    }
    if (str == "VK_FORMAT_R32G32B32_UINT") {
        return 104;
    }
    if (str == "VK_FORMAT_R32G32B32_SINT") {
        return 105;
    }
    if (str == "VK_FORMAT_R32G32B32_SFLOAT") {
        return 106;
    }
    if (str == "VK_FORMAT_R32G32B32A32_UINT") {
        return 107;
    }
    if (str == "VK_FORMAT_R32G32B32A32_SINT") {
        return 108;
    }
    if (str == "VK_FORMAT_R32G32B32A32_SFLOAT") {
        return 109;
    }
    if (str == "VK_FORMAT_R64_UINT") {
        return 110;
    }
    if (str == "VK_FORMAT_R64_SINT") {
        return 111;
    }
    if (str == "VK_FORMAT_R64_SFLOAT") {
        return 112;
    }
    if (str == "VK_FORMAT_R64G64_UINT") {
        return 113;
    }
    if (str == "VK_FORMAT_R64G64_SINT") {
        return 114;
    }
    if (str == "VK_FORMAT_R64G64_SFLOAT") {
        return 115;
    }
    if (str == "VK_FORMAT_R64G64B64_UINT") {
        return 116;
    }
    if (str == "VK_FORMAT_R64G64B64_SINT") {
        return 117;
    }
    if (str == "VK_FORMAT_R64G64B64_SFLOAT") {
        return 118;
    }
    if (str == "VK_FORMAT_R64G64B64A64_UINT") {
        return 119;
    }
    if (str == "VK_FORMAT_R64G64B64A64_SINT") {
        return 120;
    }
    if (str == "VK_FORMAT_R64G64B64A64_SFLOAT") {
        return 121;
    }
    if (str == "VK_FORMAT_B10G11R11_UFLOAT_PACK32") {
        return 122;
    }
    if (str == "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32") {
        return 123;
    }
    if (str == "VK_FORMAT_D16_UNORM") {
        return 124;
    }
    if (str == "VK_FORMAT_X8_D24_UNORM_PACK32") {
        return 125;
    }
    if (str == "VK_FORMAT_D32_SFLOAT") {
        return 126;
    }
    if (str == "VK_FORMAT_S8_UINT") {
        return 127;
    }
    if (str == "VK_FORMAT_D16_UNORM_S8_UINT") {
        return 128;
    }
    if (str == "VK_FORMAT_D24_UNORM_S8_UINT") {
        return 129;
    }
    if (str == "VK_FORMAT_D32_SFLOAT_S8_UINT") {
        return 130;
    }
    if (str == "VK_FORMAT_BC1_RGB_UNORM_BLOCK") {
        return 131;
    }
    if (str == "VK_FORMAT_BC1_RGB_SRGB_BLOCK") {
        return 132;
    }
    if (str == "VK_FORMAT_BC1_RGBA_UNORM_BLOCK") {
        return 133;
    }
    if (str == "VK_FORMAT_BC1_RGBA_SRGB_BLOCK") {
        return 134;
    }
    if (str == "VK_FORMAT_BC2_UNORM_BLOCK") {
        return 135;
    }
    if (str == "VK_FORMAT_BC2_SRGB_BLOCK") {
        return 136;
    }
    if (str == "VK_FORMAT_BC3_UNORM_BLOCK") {
        return 137;
    }
    if (str == "VK_FORMAT_BC3_SRGB_BLOCK") {
        return 138;
    }
    if (str == "VK_FORMAT_BC4_UNORM_BLOCK") {
        return 139;
    }
    if (str == "VK_FORMAT_BC4_SNORM_BLOCK") {
        return 140;
    }
    if (str == "VK_FORMAT_BC5_UNORM_BLOCK") {
        return 141;
    }
    if (str == "VK_FORMAT_BC5_SNORM_BLOCK") {
        return 142;
    }
    if (str == "VK_FORMAT_BC6H_UFLOAT_BLOCK") {
        return 143;
    }
    if (str == "VK_FORMAT_BC6H_SFLOAT_BLOCK") {
        return 144;
    }
    if (str == "VK_FORMAT_BC7_UNORM_BLOCK") {
        return 145;
    }
    if (str == "VK_FORMAT_BC7_SRGB_BLOCK") {
        return 146;
    }
    if (str == "VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK") {
        return 147;
    }
    if (str == "VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK") {
        return 148;
    }
    if (str == "VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK") {
        return 149;
    }
    if (str == "VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK") {
        return 150;
    }
    if (str == "VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK") {
        return 151;
    }
    if (str == "VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK") {
        return 152;
    }
    if (str == "VK_FORMAT_EAC_R11_UNORM_BLOCK") {
        return 153;
    }
    if (str == "VK_FORMAT_EAC_R11_SNORM_BLOCK") {
        return 154;
    }
    if (str == "VK_FORMAT_EAC_R11G11_UNORM_BLOCK") {
        return 155;
    }
    if (str == "VK_FORMAT_EAC_R11G11_SNORM_BLOCK") {
        return 156;
    }
    if (str == "VK_FORMAT_ASTC_4x4_UNORM_BLOCK") {
        return 157;
    }
    if (str == "VK_FORMAT_ASTC_4x4_SRGB_BLOCK") {
        return 158;
    }
    if (str == "VK_FORMAT_ASTC_5x4_UNORM_BLOCK") {
        return 159;
    }
    if (str == "VK_FORMAT_ASTC_5x4_SRGB_BLOCK") {
        return 160;
    }
    if (str == "VK_FORMAT_ASTC_5x5_UNORM_BLOCK") {
        return 161;
    }
    if (str == "VK_FORMAT_ASTC_5x5_SRGB_BLOCK") {
        return 162;
    }
    if (str == "VK_FORMAT_ASTC_6x5_UNORM_BLOCK") {
        return 163;
    }
    if (str == "VK_FORMAT_ASTC_6x5_SRGB_BLOCK") {
        return 164;
    }
    if (str == "VK_FORMAT_ASTC_6x6_UNORM_BLOCK") {
        return 165;
    }
    if (str == "VK_FORMAT_ASTC_6x6_SRGB_BLOCK") {
        return 166;
    }
    if (str == "VK_FORMAT_ASTC_8x5_UNORM_BLOCK") {
        return 167;
    }
    if (str == "VK_FORMAT_ASTC_8x5_SRGB_BLOCK") {
        return 168;
    }
    if (str == "VK_FORMAT_ASTC_8x6_UNORM_BLOCK") {
        return 169;
    }
    if (str == "VK_FORMAT_ASTC_8x6_SRGB_BLOCK") {
        return 170;
    }
    if (str == "VK_FORMAT_ASTC_8x8_UNORM_BLOCK") {
        return 171;
    }
    if (str == "VK_FORMAT_ASTC_8x8_SRGB_BLOCK") {
        return 172;
    }
    if (str == "VK_FORMAT_ASTC_10x5_UNORM_BLOCK") {
        return 173;
    }
    if (str == "VK_FORMAT_ASTC_10x5_SRGB_BLOCK") {
        return 174;
    }
    if (str == "VK_FORMAT_ASTC_10x6_UNORM_BLOCK") {
        return 175;
    }
    if (str == "VK_FORMAT_ASTC_10x6_SRGB_BLOCK") {
        return 176;
    }
    if (str == "VK_FORMAT_ASTC_10x8_UNORM_BLOCK") {
        return 177;
    }
    if (str == "VK_FORMAT_ASTC_10x8_SRGB_BLOCK") {
        return 178;
    }
    if (str == "VK_FORMAT_ASTC_10x10_UNORM_BLOCK") {
        return 179;
    }
    if (str == "VK_FORMAT_ASTC_10x10_SRGB_BLOCK") {
        return 180;
    }
    if (str == "VK_FORMAT_ASTC_12x10_UNORM_BLOCK") {
        return 181;
    }
    if (str == "VK_FORMAT_ASTC_12x10_SRGB_BLOCK") {
        return 182;
    }
    if (str == "VK_FORMAT_ASTC_12x12_UNORM_BLOCK") {
        return 183;
    }
    if (str == "VK_FORMAT_ASTC_12x12_SRGB_BLOCK") {
        return 184;
    }
    if (str == "VK_FORMAT_G8B8G8R8_422_UNORM") {
        return 1000156000;
    }
    if (str == "VK_FORMAT_B8G8R8G8_422_UNORM") {
        return 1000156001;
    }
    if (str == "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM") {
        return 1000156002;
    }
    if (str == "VK_FORMAT_G8_B8R8_2PLANE_420_UNORM") {
        return 1000156003;
    }
    if (str == "VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM") {
        return 1000156004;
    }
    if (str == "VK_FORMAT_G8_B8R8_2PLANE_422_UNORM") {
        return 1000156005;
    }
    if (str == "VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM") {
        return 1000156006;
    }
    if (str == "VK_FORMAT_R10X6_UNORM_PACK16") {
        return 1000156007;
    }
    if (str == "VK_FORMAT_R10X6G10X6_UNORM_2PACK16") {
        return 1000156008;
    }
    if (str == "VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16") {
        return 1000156009;
    }
    if (str == "VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16") {
        return 1000156010;
    }
    if (str == "VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16") {
        return 1000156011;
    }
    if (str == "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16") {
        return 1000156012;
    }
    if (str == "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16") {
        return 1000156013;
    }
    if (str == "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16") {
        return 1000156014;
    }
    if (str == "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16") {
        return 1000156015;
    }
    if (str == "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16") {
        return 1000156016;
    }
    if (str == "VK_FORMAT_R12X4_UNORM_PACK16") {
        return 1000156017;
    }
    if (str == "VK_FORMAT_R12X4G12X4_UNORM_2PACK16") {
        return 1000156018;
    }
    if (str == "VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16") {
        return 1000156019;
    }
    if (str == "VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16") {
        return 1000156020;
    }
    if (str == "VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16") {
        return 1000156021;
    }
    if (str == "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16") {
        return 1000156022;
    }
    if (str == "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16") {
        return 1000156023;
    }
    if (str == "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16") {
        return 1000156024;
    }
    if (str == "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16") {
        return 1000156025;
    }
    if (str == "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16") {
        return 1000156026;
    }
    if (str == "VK_FORMAT_G16B16G16R16_422_UNORM") {
        return 1000156027;
    }
    if (str == "VK_FORMAT_B16G16R16G16_422_UNORM") {
        return 1000156028;
    }
    if (str == "VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM") {
        return 1000156029;
    }
    if (str == "VK_FORMAT_G16_B16R16_2PLANE_420_UNORM") {
        return 1000156030;
    }
    if (str == "VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM") {
        return 1000156031;
    }
    if (str == "VK_FORMAT_G16_B16R16_2PLANE_422_UNORM") {
        return 1000156032;
    }
    if (str == "VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM") {
        return 1000156033;
    }
    if (str == "VK_FORMAT_G8_B8R8_2PLANE_444_UNORM") {
        return 1000330000;
    }
    if (str == "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16") {
        return 1000330001;
    }
    if (str == "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16") {
        return 1000330002;
    }
    if (str == "VK_FORMAT_G16_B16R16_2PLANE_444_UNORM") {
        return 1000330003;
    }
    if (str == "VK_FORMAT_A4R4G4B4_UNORM_PACK16") {
        return 1000340000;
    }
    if (str == "VK_FORMAT_A4B4G4R4_UNORM_PACK16") {
        return 1000340001;
    }
    if (str == "VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK") {
        return 1000066000;
    }
    if (str == "VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK") {
        return 1000066001;
    }
    if (str == "VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK") {
        return 1000066002;
    }
    if (str == "VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK") {
        return 1000066003;
    }
    if (str == "VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK") {
        return 1000066004;
    }
    if (str == "VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK") {
        return 1000066005;
    }
    if (str == "VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK") {
        return 1000066006;
    }
    if (str == "VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK") {
        return 1000066007;
    }
    if (str == "VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK") {
        return 1000066008;
    }
    if (str == "VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK") {
        return 1000066009;
    }
    if (str == "VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK") {
        return 1000066010;
    }
    if (str == "VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK") {
        return 1000066011;
    }
    if (str == "VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK") {
        return 1000066012;
    }
    if (str == "VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK") {
        return 1000066013;
    }
    if (str == "VK_FORMAT_A1B5G5R5_UNORM_PACK16") {
        return 1000470000;
    }
    if (str == "VK_FORMAT_A8_UNORM") {
        return 1000470001;
    }
    if (str == "VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG") {
        return 1000054000;
    }
    if (str == "VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG") {
        return 1000054001;
    }
    if (str == "VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG") {
        return 1000054002;
    }
    if (str == "VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG") {
        return 1000054003;
    }
    if (str == "VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG") {
        return 1000054004;
    }
    if (str == "VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG") {
        return 1000054005;
    }
    if (str == "VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG") {
        return 1000054006;
    }
    if (str == "VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG") {
        return 1000054007;
    }
    if (str == "VK_FORMAT_SBS80_ARM") {
        return 1000452000;
    }
    if (str == "VK_FORMAT_R8_BOOL_ARM") {
        return 1000460000;
    }
    if (str == "VK_FORMAT_R16G16_SFIXED5_NV") {
        return 1000464000;
    }
    if (str == "VK_FORMAT_MAX_ENUM") {
        return 0x7FFFFFFF;
    }
    // Unknown name
    return -1;
}

// Validate that the decoded enum is valid given the Vulkan headers version in use. If this call
// fails, it indicates that either a newer vulkan_core.h was used by the VGF generator OR that the
// VGF is corrupted in some other way. Regenerating this header file with a newer Vulkan headers
// version may resolve the issue.
inline bool ValidateDecodedFormatType(FormatType e) {
    switch (static_cast<int>(e)) {
    case 0:          // VK_FORMAT_UNDEFINED
    case 1:          // VK_FORMAT_R4G4_UNORM_PACK8
    case 2:          // VK_FORMAT_R4G4B4A4_UNORM_PACK16
    case 3:          // VK_FORMAT_B4G4R4A4_UNORM_PACK16
    case 4:          // VK_FORMAT_R5G6B5_UNORM_PACK16
    case 5:          // VK_FORMAT_B5G6R5_UNORM_PACK16
    case 6:          // VK_FORMAT_R5G5B5A1_UNORM_PACK16
    case 7:          // VK_FORMAT_B5G5R5A1_UNORM_PACK16
    case 8:          // VK_FORMAT_A1R5G5B5_UNORM_PACK16
    case 9:          // VK_FORMAT_R8_UNORM
    case 10:         // VK_FORMAT_R8_SNORM
    case 11:         // VK_FORMAT_R8_USCALED
    case 12:         // VK_FORMAT_R8_SSCALED
    case 13:         // VK_FORMAT_R8_UINT
    case 14:         // VK_FORMAT_R8_SINT
    case 15:         // VK_FORMAT_R8_SRGB
    case 16:         // VK_FORMAT_R8G8_UNORM
    case 17:         // VK_FORMAT_R8G8_SNORM
    case 18:         // VK_FORMAT_R8G8_USCALED
    case 19:         // VK_FORMAT_R8G8_SSCALED
    case 20:         // VK_FORMAT_R8G8_UINT
    case 21:         // VK_FORMAT_R8G8_SINT
    case 22:         // VK_FORMAT_R8G8_SRGB
    case 23:         // VK_FORMAT_R8G8B8_UNORM
    case 24:         // VK_FORMAT_R8G8B8_SNORM
    case 25:         // VK_FORMAT_R8G8B8_USCALED
    case 26:         // VK_FORMAT_R8G8B8_SSCALED
    case 27:         // VK_FORMAT_R8G8B8_UINT
    case 28:         // VK_FORMAT_R8G8B8_SINT
    case 29:         // VK_FORMAT_R8G8B8_SRGB
    case 30:         // VK_FORMAT_B8G8R8_UNORM
    case 31:         // VK_FORMAT_B8G8R8_SNORM
    case 32:         // VK_FORMAT_B8G8R8_USCALED
    case 33:         // VK_FORMAT_B8G8R8_SSCALED
    case 34:         // VK_FORMAT_B8G8R8_UINT
    case 35:         // VK_FORMAT_B8G8R8_SINT
    case 36:         // VK_FORMAT_B8G8R8_SRGB
    case 37:         // VK_FORMAT_R8G8B8A8_UNORM
    case 38:         // VK_FORMAT_R8G8B8A8_SNORM
    case 39:         // VK_FORMAT_R8G8B8A8_USCALED
    case 40:         // VK_FORMAT_R8G8B8A8_SSCALED
    case 41:         // VK_FORMAT_R8G8B8A8_UINT
    case 42:         // VK_FORMAT_R8G8B8A8_SINT
    case 43:         // VK_FORMAT_R8G8B8A8_SRGB
    case 44:         // VK_FORMAT_B8G8R8A8_UNORM
    case 45:         // VK_FORMAT_B8G8R8A8_SNORM
    case 46:         // VK_FORMAT_B8G8R8A8_USCALED
    case 47:         // VK_FORMAT_B8G8R8A8_SSCALED
    case 48:         // VK_FORMAT_B8G8R8A8_UINT
    case 49:         // VK_FORMAT_B8G8R8A8_SINT
    case 50:         // VK_FORMAT_B8G8R8A8_SRGB
    case 51:         // VK_FORMAT_A8B8G8R8_UNORM_PACK32
    case 52:         // VK_FORMAT_A8B8G8R8_SNORM_PACK32
    case 53:         // VK_FORMAT_A8B8G8R8_USCALED_PACK32
    case 54:         // VK_FORMAT_A8B8G8R8_SSCALED_PACK32
    case 55:         // VK_FORMAT_A8B8G8R8_UINT_PACK32
    case 56:         // VK_FORMAT_A8B8G8R8_SINT_PACK32
    case 57:         // VK_FORMAT_A8B8G8R8_SRGB_PACK32
    case 58:         // VK_FORMAT_A2R10G10B10_UNORM_PACK32
    case 59:         // VK_FORMAT_A2R10G10B10_SNORM_PACK32
    case 60:         // VK_FORMAT_A2R10G10B10_USCALED_PACK32
    case 61:         // VK_FORMAT_A2R10G10B10_SSCALED_PACK32
    case 62:         // VK_FORMAT_A2R10G10B10_UINT_PACK32
    case 63:         // VK_FORMAT_A2R10G10B10_SINT_PACK32
    case 64:         // VK_FORMAT_A2B10G10R10_UNORM_PACK32
    case 65:         // VK_FORMAT_A2B10G10R10_SNORM_PACK32
    case 66:         // VK_FORMAT_A2B10G10R10_USCALED_PACK32
    case 67:         // VK_FORMAT_A2B10G10R10_SSCALED_PACK32
    case 68:         // VK_FORMAT_A2B10G10R10_UINT_PACK32
    case 69:         // VK_FORMAT_A2B10G10R10_SINT_PACK32
    case 70:         // VK_FORMAT_R16_UNORM
    case 71:         // VK_FORMAT_R16_SNORM
    case 72:         // VK_FORMAT_R16_USCALED
    case 73:         // VK_FORMAT_R16_SSCALED
    case 74:         // VK_FORMAT_R16_UINT
    case 75:         // VK_FORMAT_R16_SINT
    case 76:         // VK_FORMAT_R16_SFLOAT
    case 77:         // VK_FORMAT_R16G16_UNORM
    case 78:         // VK_FORMAT_R16G16_SNORM
    case 79:         // VK_FORMAT_R16G16_USCALED
    case 80:         // VK_FORMAT_R16G16_SSCALED
    case 81:         // VK_FORMAT_R16G16_UINT
    case 82:         // VK_FORMAT_R16G16_SINT
    case 83:         // VK_FORMAT_R16G16_SFLOAT
    case 84:         // VK_FORMAT_R16G16B16_UNORM
    case 85:         // VK_FORMAT_R16G16B16_SNORM
    case 86:         // VK_FORMAT_R16G16B16_USCALED
    case 87:         // VK_FORMAT_R16G16B16_SSCALED
    case 88:         // VK_FORMAT_R16G16B16_UINT
    case 89:         // VK_FORMAT_R16G16B16_SINT
    case 90:         // VK_FORMAT_R16G16B16_SFLOAT
    case 91:         // VK_FORMAT_R16G16B16A16_UNORM
    case 92:         // VK_FORMAT_R16G16B16A16_SNORM
    case 93:         // VK_FORMAT_R16G16B16A16_USCALED
    case 94:         // VK_FORMAT_R16G16B16A16_SSCALED
    case 95:         // VK_FORMAT_R16G16B16A16_UINT
    case 96:         // VK_FORMAT_R16G16B16A16_SINT
    case 97:         // VK_FORMAT_R16G16B16A16_SFLOAT
    case 98:         // VK_FORMAT_R32_UINT
    case 99:         // VK_FORMAT_R32_SINT
    case 100:        // VK_FORMAT_R32_SFLOAT
    case 101:        // VK_FORMAT_R32G32_UINT
    case 102:        // VK_FORMAT_R32G32_SINT
    case 103:        // VK_FORMAT_R32G32_SFLOAT
    case 104:        // VK_FORMAT_R32G32B32_UINT
    case 105:        // VK_FORMAT_R32G32B32_SINT
    case 106:        // VK_FORMAT_R32G32B32_SFLOAT
    case 107:        // VK_FORMAT_R32G32B32A32_UINT
    case 108:        // VK_FORMAT_R32G32B32A32_SINT
    case 109:        // VK_FORMAT_R32G32B32A32_SFLOAT
    case 110:        // VK_FORMAT_R64_UINT
    case 111:        // VK_FORMAT_R64_SINT
    case 112:        // VK_FORMAT_R64_SFLOAT
    case 113:        // VK_FORMAT_R64G64_UINT
    case 114:        // VK_FORMAT_R64G64_SINT
    case 115:        // VK_FORMAT_R64G64_SFLOAT
    case 116:        // VK_FORMAT_R64G64B64_UINT
    case 117:        // VK_FORMAT_R64G64B64_SINT
    case 118:        // VK_FORMAT_R64G64B64_SFLOAT
    case 119:        // VK_FORMAT_R64G64B64A64_UINT
    case 120:        // VK_FORMAT_R64G64B64A64_SINT
    case 121:        // VK_FORMAT_R64G64B64A64_SFLOAT
    case 122:        // VK_FORMAT_B10G11R11_UFLOAT_PACK32
    case 123:        // VK_FORMAT_E5B9G9R9_UFLOAT_PACK32
    case 124:        // VK_FORMAT_D16_UNORM
    case 125:        // VK_FORMAT_X8_D24_UNORM_PACK32
    case 126:        // VK_FORMAT_D32_SFLOAT
    case 127:        // VK_FORMAT_S8_UINT
    case 128:        // VK_FORMAT_D16_UNORM_S8_UINT
    case 129:        // VK_FORMAT_D24_UNORM_S8_UINT
    case 130:        // VK_FORMAT_D32_SFLOAT_S8_UINT
    case 131:        // VK_FORMAT_BC1_RGB_UNORM_BLOCK
    case 132:        // VK_FORMAT_BC1_RGB_SRGB_BLOCK
    case 133:        // VK_FORMAT_BC1_RGBA_UNORM_BLOCK
    case 134:        // VK_FORMAT_BC1_RGBA_SRGB_BLOCK
    case 135:        // VK_FORMAT_BC2_UNORM_BLOCK
    case 136:        // VK_FORMAT_BC2_SRGB_BLOCK
    case 137:        // VK_FORMAT_BC3_UNORM_BLOCK
    case 138:        // VK_FORMAT_BC3_SRGB_BLOCK
    case 139:        // VK_FORMAT_BC4_UNORM_BLOCK
    case 140:        // VK_FORMAT_BC4_SNORM_BLOCK
    case 141:        // VK_FORMAT_BC5_UNORM_BLOCK
    case 142:        // VK_FORMAT_BC5_SNORM_BLOCK
    case 143:        // VK_FORMAT_BC6H_UFLOAT_BLOCK
    case 144:        // VK_FORMAT_BC6H_SFLOAT_BLOCK
    case 145:        // VK_FORMAT_BC7_UNORM_BLOCK
    case 146:        // VK_FORMAT_BC7_SRGB_BLOCK
    case 147:        // VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK
    case 148:        // VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK
    case 149:        // VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK
    case 150:        // VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK
    case 151:        // VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK
    case 152:        // VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK
    case 153:        // VK_FORMAT_EAC_R11_UNORM_BLOCK
    case 154:        // VK_FORMAT_EAC_R11_SNORM_BLOCK
    case 155:        // VK_FORMAT_EAC_R11G11_UNORM_BLOCK
    case 156:        // VK_FORMAT_EAC_R11G11_SNORM_BLOCK
    case 157:        // VK_FORMAT_ASTC_4x4_UNORM_BLOCK
    case 158:        // VK_FORMAT_ASTC_4x4_SRGB_BLOCK
    case 159:        // VK_FORMAT_ASTC_5x4_UNORM_BLOCK
    case 160:        // VK_FORMAT_ASTC_5x4_SRGB_BLOCK
    case 161:        // VK_FORMAT_ASTC_5x5_UNORM_BLOCK
    case 162:        // VK_FORMAT_ASTC_5x5_SRGB_BLOCK
    case 163:        // VK_FORMAT_ASTC_6x5_UNORM_BLOCK
    case 164:        // VK_FORMAT_ASTC_6x5_SRGB_BLOCK
    case 165:        // VK_FORMAT_ASTC_6x6_UNORM_BLOCK
    case 166:        // VK_FORMAT_ASTC_6x6_SRGB_BLOCK
    case 167:        // VK_FORMAT_ASTC_8x5_UNORM_BLOCK
    case 168:        // VK_FORMAT_ASTC_8x5_SRGB_BLOCK
    case 169:        // VK_FORMAT_ASTC_8x6_UNORM_BLOCK
    case 170:        // VK_FORMAT_ASTC_8x6_SRGB_BLOCK
    case 171:        // VK_FORMAT_ASTC_8x8_UNORM_BLOCK
    case 172:        // VK_FORMAT_ASTC_8x8_SRGB_BLOCK
    case 173:        // VK_FORMAT_ASTC_10x5_UNORM_BLOCK
    case 174:        // VK_FORMAT_ASTC_10x5_SRGB_BLOCK
    case 175:        // VK_FORMAT_ASTC_10x6_UNORM_BLOCK
    case 176:        // VK_FORMAT_ASTC_10x6_SRGB_BLOCK
    case 177:        // VK_FORMAT_ASTC_10x8_UNORM_BLOCK
    case 178:        // VK_FORMAT_ASTC_10x8_SRGB_BLOCK
    case 179:        // VK_FORMAT_ASTC_10x10_UNORM_BLOCK
    case 180:        // VK_FORMAT_ASTC_10x10_SRGB_BLOCK
    case 181:        // VK_FORMAT_ASTC_12x10_UNORM_BLOCK
    case 182:        // VK_FORMAT_ASTC_12x10_SRGB_BLOCK
    case 183:        // VK_FORMAT_ASTC_12x12_UNORM_BLOCK
    case 184:        // VK_FORMAT_ASTC_12x12_SRGB_BLOCK
    case 1000156000: // VK_FORMAT_G8B8G8R8_422_UNORM / VK_FORMAT_G8B8G8R8_422_UNORM_KHR
    case 1000156001: // VK_FORMAT_B8G8R8G8_422_UNORM / VK_FORMAT_B8G8R8G8_422_UNORM_KHR
    case 1000156002: // VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM / VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR
    case 1000156003: // VK_FORMAT_G8_B8R8_2PLANE_420_UNORM / VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR
    case 1000156004: // VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM / VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR
    case 1000156005: // VK_FORMAT_G8_B8R8_2PLANE_422_UNORM / VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR
    case 1000156006: // VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM / VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR
    case 1000156007: // VK_FORMAT_R10X6_UNORM_PACK16 / VK_FORMAT_R10X6_UNORM_PACK16_KHR
    case 1000156008: // VK_FORMAT_R10X6G10X6_UNORM_2PACK16 / VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR
    case 1000156009: // VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16 / VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR
    case 1000156010: // VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16 /
                     // VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR
    case 1000156011: // VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16 /
                     // VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR
    case 1000156012: // VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 /
                     // VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR
    case 1000156013: // VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16 /
                     // VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR
    case 1000156014: // VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 /
                     // VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR
    case 1000156015: // VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16 /
                     // VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR
    case 1000156016: // VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 /
                     // VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR
    case 1000156017: // VK_FORMAT_R12X4_UNORM_PACK16 / VK_FORMAT_R12X4_UNORM_PACK16_KHR
    case 1000156018: // VK_FORMAT_R12X4G12X4_UNORM_2PACK16 / VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR
    case 1000156019: // VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16 / VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR
    case 1000156020: // VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16 /
                     // VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR
    case 1000156021: // VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16 /
                     // VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR
    case 1000156022: // VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 /
                     // VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR
    case 1000156023: // VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16 /
                     // VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR
    case 1000156024: // VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 /
                     // VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR
    case 1000156025: // VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16 /
                     // VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR
    case 1000156026: // VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 /
                     // VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR
    case 1000156027: // VK_FORMAT_G16B16G16R16_422_UNORM / VK_FORMAT_G16B16G16R16_422_UNORM_KHR
    case 1000156028: // VK_FORMAT_B16G16R16G16_422_UNORM / VK_FORMAT_B16G16R16G16_422_UNORM_KHR
    case 1000156029: // VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM / VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR
    case 1000156030: // VK_FORMAT_G16_B16R16_2PLANE_420_UNORM / VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR
    case 1000156031: // VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM / VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR
    case 1000156032: // VK_FORMAT_G16_B16R16_2PLANE_422_UNORM / VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR
    case 1000156033: // VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM / VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR
    case 1000330000: // VK_FORMAT_G8_B8R8_2PLANE_444_UNORM / VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT
    case 1000330001: // VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16 /
                     // VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT
    case 1000330002: // VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16 /
                     // VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT
    case 1000330003: // VK_FORMAT_G16_B16R16_2PLANE_444_UNORM / VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT
    case 1000340000: // VK_FORMAT_A4R4G4B4_UNORM_PACK16 / VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT
    case 1000340001: // VK_FORMAT_A4B4G4R4_UNORM_PACK16 / VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT
    case 1000066000: // VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK / VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT
    case 1000066001: // VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK / VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT
    case 1000066002: // VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK / VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT
    case 1000066003: // VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK / VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT
    case 1000066004: // VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK / VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT
    case 1000066005: // VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK / VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT
    case 1000066006: // VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK / VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT
    case 1000066007: // VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK / VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT
    case 1000066008: // VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK / VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT
    case 1000066009: // VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK / VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT
    case 1000066010: // VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK / VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT
    case 1000066011: // VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK / VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT
    case 1000066012: // VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK / VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT
    case 1000066013: // VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK / VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT
    case 1000470000: // VK_FORMAT_A1B5G5R5_UNORM_PACK16 / VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR
    case 1000470001: // VK_FORMAT_A8_UNORM / VK_FORMAT_A8_UNORM_KHR
    case 1000054000: // VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG
    case 1000054001: // VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG
    case 1000054002: // VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG
    case 1000054003: // VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG
    case 1000054004: // VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG
    case 1000054005: // VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG
    case 1000054006: // VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG
    case 1000054007: // VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG
    case 1000452000: // VK_FORMAT_SBS80_ARM
    case 1000460000: // VK_FORMAT_R8_BOOL_ARM
    case 1000464000: // VK_FORMAT_R16G16_SFIXED5_NV / VK_FORMAT_R16G16_S10_5_NV
    case 0x7FFFFFFF: // VK_FORMAT_MAX_ENUM
        return true;
    default:
        return false;
    }
}

// Get the texel block size in bytes of the given format.
inline uint8_t blockSize(FormatType e) {
    switch (static_cast<int>(e)) {
    case 9:
        return 1;
    case 10:
        return 1;
    case 11:
        return 1;
    case 12:
        return 1;
    case 13:
        return 1;
    case 14:
        return 1;
    case 15:
        return 1;
    case 70:
        return 2;
    case 71:
        return 2;
    case 72:
        return 2;
    case 73:
        return 2;
    case 74:
        return 2;
    case 75:
        return 2;
    case 76:
        return 2;
    case 98:
        return 4;
    case 99:
        return 4;
    case 100:
        return 4;
    case 110:
        return 8;
    case 111:
        return 8;
    case 112:
        return 8;
    case 124:
        return 2;
    case 125:
        return 4;
    case 126:
        return 4;
    case 127:
        return 1;
    case 139:
        return 8;
    case 140:
        return 8;
    case 153:
        return 8;
    case 154:
        return 8;
    case 1000156007:
        return 2;
    case 1000156017:
        return 2;
    case 1000470001:
        return 1;
    case 1000452000:
        return 10;
    case 1000460000:
        return 1;

    default:
        return 0;
    }
}

// Get the numeric format of the given format.
inline std::string componentNumericFormat(FormatType e) {
    switch (static_cast<int>(e)) {
    case 9:
        return "UNORM";
    case 10:
        return "SNORM";
    case 11:
        return "USCALED";
    case 12:
        return "SSCALED";
    case 13:
        return "UINT";
    case 14:
        return "SINT";
    case 15:
        return "SRGB";
    case 70:
        return "UNORM";
    case 71:
        return "SNORM";
    case 72:
        return "USCALED";
    case 73:
        return "SSCALED";
    case 74:
        return "UINT";
    case 75:
        return "SINT";
    case 76:
        return "SFLOAT";
    case 98:
        return "UINT";
    case 99:
        return "SINT";
    case 100:
        return "SFLOAT";
    case 110:
        return "UINT";
    case 111:
        return "SINT";
    case 112:
        return "SFLOAT";
    case 124:
        return "UNORM";
    case 125:
        return "UNORM";
    case 126:
        return "SFLOAT";
    case 127:
        return "UINT";
    case 139:
        return "UNORM";
    case 140:
        return "SNORM";
    case 153:
        return "UNORM";
    case 154:
        return "SNORM";
    case 1000156007:
        return "UNORM";
    case 1000156017:
        return "UNORM";
    case 1000470001:
        return "UNORM";
    case 1000452000:
        return "UINT";
    case 1000460000:
        return "BOOL";

    default:
        return "";
    }
}

} // namespace mlsdk::vgflib
