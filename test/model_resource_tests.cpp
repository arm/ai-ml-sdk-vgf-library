/*
 * SPDX-FileCopyrightText: Copyright 2023-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf/decoder.h"
#include "vgf/decoder.hpp"
#include "vgf/encoder.hpp"
#include "vgf/types.hpp"

#include "header.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <string>

using namespace mlsdk::vgflib;

namespace {
const uint16_t pretendVulkanHeaderVersion = 123;

constexpr bool DataViewTests() {
    static_assert(DataView<uint8_t>().empty(), "Default constructor should create an empty view");
    static_assert(DataView<uint8_t>().size() == 0, "Default constructor should create a view of size 0");
    static_assert(DataView<uint8_t>(nullptr, 0).empty(),
                  "Constructor with nullptr and size 0 should create an empty view");
    static_assert(DataView<uint32_t>() == DataView<uint32_t>(nullptr, 0),
                  "Two DataViews constructed with nullptr and size 0 should be equal");

    int i = 42;
    DataView<int> dv(&i, 1);
    return dv[0] == i;
}

} // namespace

TEST(DataView, Basic) {
    static_assert(DataViewTests(), "DataView tests");
    ASSERT_TRUE(DataViewTests());
}

TEST(CppModelResourceTable, EmptyTable) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder =
        CreateHeaderDecoder(data.c_str(), static_cast<uint64_t>(data.size()));
    ASSERT_TRUE(headerDecoder->IsValid());
    ASSERT_TRUE(headerDecoder->CheckVersion());

    ASSERT_TRUE(VerifyModelResourceTable(data.c_str() + headerDecoder->GetModelResourceTableOffset(),
                                         headerDecoder->GetModelResourceTableSize()));
    std::unique_ptr<ModelResourceTableDecoder> decoder = CreateModelResourceTableDecoder(
        data.c_str() + headerDecoder->GetModelResourceTableOffset(), headerDecoder->GetModelResourceTableSize());

    ASSERT_TRUE(decoder->size() == 0);
}

TEST(CppModelResourceTable, EncodeDecode) {
    std::stringstream buffer;

    //! [MrtEncodeInput begin]
    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    std::vector<int64_t> shape1{0, 1, 2, 3};
    std::vector<int64_t> strides1{4, 5, 6, 7};

    DescriptorType VK_DESCRIPTOR_TYPE_STORAGE_IMAGE = 3;
    FormatType VK_FORMAT_R4G4_UNORM_PACK8 = 1;
    ResourceRef resource0 =
        encoder->AddInputResource(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R4G4_UNORM_PACK8, shape1, strides1);
    ResourceRef resource1 =
        encoder->AddOutputResource(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R4G4_UNORM_PACK8, shape1, strides1);
    //! [MrtEncodeInput end]
    (void)resource1;

    //! [MrtEncodeConstant begin]
    FormatType VK_FORMAT_R4G4B4A4_UNORM_PACK16 = 12;
    std::vector<int64_t> shape2{8, 9, 10, 11};
    std::vector<int64_t> strides2{12, 13, 14, 15};
    ResourceRef resource2 = encoder->AddConstantResource(VK_FORMAT_R4G4B4A4_UNORM_PACK16, shape2, strides2);
    //! [MrtEncodeConstant end]

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string vgf_data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder =
        CreateHeaderDecoder(vgf_data.c_str(), static_cast<uint64_t>(vgf_data.size()));
    ASSERT_TRUE(headerDecoder->IsValid());
    ASSERT_TRUE(headerDecoder->CheckVersion());

    uint32_t mrtIndex = resource0.reference;
    //! [MrtDecodingSample0 begin]
    ASSERT_TRUE(VerifyModelResourceTable(vgf_data.c_str() + headerDecoder->GetModelResourceTableOffset(),
                                         headerDecoder->GetModelResourceTableSize()));
    std::unique_ptr<ModelResourceTableDecoder> mrtDecoder = CreateModelResourceTableDecoder(
        vgf_data.c_str() + headerDecoder->GetModelResourceTableOffset(), headerDecoder->GetModelResourceTableSize());

    size_t numEntries = mrtDecoder->size();

    ASSERT_TRUE(numEntries > mrtIndex);

    ResourceCategory category = mrtDecoder->getCategory(mrtIndex);
    DataView<int64_t> shape = mrtDecoder->getTensorShape(mrtIndex);
    ASSERT_FALSE(shape.empty());
    std::optional<DescriptorType> type = mrtDecoder->getDescriptorType(mrtIndex);
    FormatType format = mrtDecoder->getVkFormat(mrtIndex);
    DataView<int64_t> stride = mrtDecoder->getTensorStride(mrtIndex);
    ASSERT_FALSE(stride.empty());
    // ...
    //! [MrtDecodingSample0 end]

    ASSERT_TRUE(numEntries == 3);
    ASSERT_TRUE(category == ResourceCategory::INPUT);
    ASSERT_TRUE(shape == DataView<int64_t>(shape1.data(), shape1.size()));
    ASSERT_TRUE(type.has_value());
    ASSERT_TRUE(type.value() == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    ASSERT_TRUE(format == VK_FORMAT_R4G4_UNORM_PACK8);
    ASSERT_TRUE(stride == DataView<int64_t>(strides1.data(), strides1.size()));

    ASSERT_TRUE(mrtDecoder->getCategory(resource2.reference) == ResourceCategory::CONSTANT);
    ASSERT_TRUE(mrtDecoder->getDescriptorType(resource2.reference).has_value() == false);
    ASSERT_TRUE(mrtDecoder->getVkFormat(resource2.reference) == VK_FORMAT_R4G4B4A4_UNORM_PACK16);
    ASSERT_TRUE(mrtDecoder->getTensorShape(resource2.reference) == DataView<int64_t>(shape2.data(), shape2.size()));
    ASSERT_TRUE(mrtDecoder->getTensorStride(resource2.reference) ==
                DataView<int64_t>(strides2.data(), strides2.size()));
}

TEST(CppModelResourceTable, UnknownDimensions) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    std::vector<int64_t> shape1{-1, -1, -1, -1};
    std::vector<int64_t> shape2{3, -1, 1, -1};

    DescriptorType VK_DESCRIPTOR_TYPE_STORAGE_IMAGE = 3;
    FormatType VK_FORMAT_R4G4_UNORM_PACK8 = 1;
    FormatType VK_FORMAT_R4G4B4A4_UNORM_PACK16 = 12;

    auto resource0 =
        encoder->AddInputResource(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R4G4_UNORM_PACK8, shape1, {});
    [[maybe_unused]] auto resource1 = encoder->AddConstantResource(VK_FORMAT_R4G4B4A4_UNORM_PACK16, shape2, {});
    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string vgf_data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder =
        CreateHeaderDecoder(vgf_data.c_str(), static_cast<uint64_t>(vgf_data.size()));
    ASSERT_TRUE(headerDecoder->IsValid());
    ASSERT_TRUE(headerDecoder->CheckVersion());

    ASSERT_TRUE(VerifyModelResourceTable(vgf_data.c_str() + headerDecoder->GetModelResourceTableOffset(),
                                         headerDecoder->GetModelResourceTableSize()));
    uint32_t mrtIndex = resource0.reference;
    std::unique_ptr<ModelResourceTableDecoder> mrtDecoder = CreateModelResourceTableDecoder(
        vgf_data.c_str() + headerDecoder->GetModelResourceTableOffset(), headerDecoder->GetModelResourceTableSize());

    size_t numEntries = mrtDecoder->size();

    ASSERT_TRUE(numEntries > mrtIndex);

    ResourceCategory category = mrtDecoder->getCategory(mrtIndex);
    DataView<int64_t> shape = mrtDecoder->getTensorShape(mrtIndex);
    ASSERT_FALSE(shape.empty());
    std::optional<DescriptorType> type = mrtDecoder->getDescriptorType(mrtIndex);
    FormatType format = mrtDecoder->getVkFormat(mrtIndex);

    ASSERT_TRUE(numEntries == 2);
    ASSERT_TRUE(category == ResourceCategory::INPUT);
    ASSERT_TRUE(shape == DataView<int64_t>(shape1.data(), shape1.size()));
    ASSERT_TRUE(type.has_value());
    ASSERT_TRUE(type.value() == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    ASSERT_TRUE(format == VK_FORMAT_R4G4_UNORM_PACK8);

    ASSERT_TRUE(mrtDecoder->getCategory(resource1.reference) == ResourceCategory::CONSTANT);
    ASSERT_TRUE(mrtDecoder->getDescriptorType(resource1.reference).has_value() == false);
    ASSERT_TRUE(mrtDecoder->getVkFormat(resource1.reference) == VK_FORMAT_R4G4B4A4_UNORM_PACK16);
    ASSERT_TRUE(mrtDecoder->getTensorShape(resource1.reference) == DataView<int64_t>(shape2.data(), shape2.size()));
}

TEST(CModelResourceTable, EmptyTable) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();

    ASSERT_TRUE(data.size() >= mlsdk_decoder_header_size());

    std::vector<uint8_t> headerDecoderMemory;
    headerDecoderMemory.resize(mlsdk_decoder_header_decoder_mem_reqs());
    mlsdk_decoder_header_decoder *headerDecoder = mlsdk_decoder_create_header_decoder(
        data.c_str(), static_cast<uint64_t>(data.size()), headerDecoderMemory.data());
    ASSERT_TRUE(mlsdk_decoder_is_header_valid(headerDecoder));
    ASSERT_TRUE(mlsdk_decoder_is_header_compatible(headerDecoder));

    mlsdk_decoder_vgf_section_info section;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_resources, &section);
    ASSERT_TRUE(mlsdk_decoder_is_valid_model_resource_table(data.c_str() + section.offset, section.size));

    std::vector<uint8_t> resourceTableMemory;
    resourceTableMemory.resize(mlsdk_decoder_model_resource_table_decoder_mem_reqs());
    mlsdk_decoder_model_resource_table_decoder *decoder = mlsdk_decoder_create_model_resource_table_decoder(
        data.c_str() + section.offset, section.size, resourceTableMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_model_resource_table_num_entries(decoder) == 0);
}

TEST(CModelResourceTable, EncodeDecode) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    std::vector<int64_t> shape1{0, 1, 2, 3};
    std::vector<int64_t> strides1{4, 5, 6, 7};
    std::vector<int64_t> shape2{8, 9, 10, 11};
    std::vector<int64_t> strides2{12, 13, 14, 15};

    DescriptorType VK_DESCRIPTOR_TYPE_STORAGE_IMAGE = 3;
    mlsdk_vk_descriptor_type mlsdk_vk_descriptor_type_storage_image = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    FormatType VK_FORMAT_R4G4_UNORM_PACK8 = 1;
    FormatType VK_FORMAT_R4G4B4A4_UNORM_PACK16 = 12;
    mlsdk_vk_format mlsdk_vk_format_r4g4_unorm_pack8 = VK_FORMAT_R4G4_UNORM_PACK8;
    mlsdk_vk_format mlsdk_vk_format_r4g4b4a4_unorm_pack16 = VK_FORMAT_R4G4B4A4_UNORM_PACK16;

    auto resource0 =
        encoder->AddInputResource(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R4G4_UNORM_PACK8, shape1, strides1);
    auto resource1 = encoder->AddConstantResource(VK_FORMAT_R4G4B4A4_UNORM_PACK16, shape2, strides2);
    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();

    std::vector<uint8_t> headerDecoderMemory;
    headerDecoderMemory.resize(mlsdk_decoder_header_decoder_mem_reqs());
    mlsdk_decoder_header_decoder *headerDecoder = mlsdk_decoder_create_header_decoder(
        data.c_str(), static_cast<uint64_t>(data.size()), headerDecoderMemory.data());
    ASSERT_TRUE(mlsdk_decoder_is_header_valid(headerDecoder));
    ASSERT_TRUE(mlsdk_decoder_is_header_compatible(headerDecoder));

    mlsdk_decoder_vgf_section_info section;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_resources, &section);
    ASSERT_TRUE(mlsdk_decoder_is_valid_model_resource_table(data.c_str() + section.offset, section.size));

    std::vector<uint8_t> resourceTableDecoderMemory;
    resourceTableDecoderMemory.resize(mlsdk_decoder_model_resource_table_decoder_mem_reqs());
    mlsdk_decoder_model_resource_table_decoder *resourceTableDecoder =
        mlsdk_decoder_create_model_resource_table_decoder(data.c_str() + section.offset, section.size,
                                                          resourceTableDecoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_model_resource_table_num_entries(resourceTableDecoder) == 2);

    ASSERT_TRUE(mlsdk_decoder_model_resource_table_get_category(resourceTableDecoder, resource0.reference) ==
                mlsdk_decoder_mrt_category_input);
    ASSERT_TRUE(mlsdk_decoder_get_vk_descriptor_type(resourceTableDecoder, resource0.reference).has_value);
    ASSERT_TRUE(mlsdk_decoder_get_vk_descriptor_type(resourceTableDecoder, resource0.reference).value ==
                mlsdk_vk_descriptor_type_storage_image);
    ASSERT_TRUE(mlsdk_decoder_get_vk_format(resourceTableDecoder, resource0.reference) ==
                mlsdk_vk_format_r4g4_unorm_pack8);

    mlsdk_decoder_tensor_dimensions tensorShape1;
    mlsdk_decoder_model_resource_table_get_tensor_shape(resourceTableDecoder, resource0.reference, &tensorShape1);
    ASSERT_TRUE(DataView<int64_t>(tensorShape1.data, tensorShape1.size) ==
                DataView<int64_t>(shape1.data(), shape1.size()));

    mlsdk_decoder_tensor_dimensions tensorStride1;
    mlsdk_decoder_model_resource_table_get_tensor_strides(resourceTableDecoder, resource0.reference, &tensorStride1);
    ASSERT_TRUE(DataView<int64_t>(tensorStride1.data, tensorStride1.size) ==
                DataView<int64_t>(strides1.data(), strides1.size()));

    ASSERT_TRUE(mlsdk_decoder_model_resource_table_get_category(resourceTableDecoder, resource1.reference) ==
                mlsdk_decoder_mrt_category_constant);
    ASSERT_TRUE(mlsdk_decoder_get_vk_descriptor_type(resourceTableDecoder, resource1.reference).has_value == false);
    ASSERT_TRUE(mlsdk_decoder_get_vk_format(resourceTableDecoder, resource1.reference) ==
                mlsdk_vk_format_r4g4b4a4_unorm_pack16);

    mlsdk_decoder_tensor_dimensions tensorShape2;
    mlsdk_decoder_model_resource_table_get_tensor_shape(resourceTableDecoder, resource1.reference, &tensorShape2);
    ASSERT_TRUE(DataView<int64_t>(tensorShape2.data, tensorShape2.size) ==
                DataView<int64_t>(shape2.data(), shape2.size()));

    mlsdk_decoder_tensor_dimensions tensorStride2;
    mlsdk_decoder_model_resource_table_get_tensor_strides(resourceTableDecoder, resource1.reference, &tensorStride2);
    ASSERT_TRUE(DataView<int64_t>(tensorStride2.data, tensorStride2.size) ==
                DataView<int64_t>(strides2.data(), strides2.size()));
}

TEST(CModelResourceTable, UnknownDimensions) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    std::vector<int64_t> shape1{-1, -1, -1, -1};
    std::vector<int64_t> shape2{8, -1, -1, 11};

    DescriptorType VK_DESCRIPTOR_TYPE_STORAGE_IMAGE = 3;
    mlsdk_vk_descriptor_type mlsdk_vk_descriptor_type_storage_image = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    FormatType VK_FORMAT_R4G4_UNORM_PACK8 = 1;
    FormatType VK_FORMAT_R4G4B4A4_UNORM_PACK16 = 12;
    mlsdk_vk_format mlsdk_vk_format_r4g4_unorm_pack8 = VK_FORMAT_R4G4_UNORM_PACK8;
    mlsdk_vk_format mlsdk_vk_format_r4g4b4a4_unorm_pack16 = VK_FORMAT_R4G4B4A4_UNORM_PACK16;

    auto resource0 =
        encoder->AddInputResource(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R4G4_UNORM_PACK8, shape1, {});
    auto resource1 = encoder->AddConstantResource(VK_FORMAT_R4G4B4A4_UNORM_PACK16, shape2, {});
    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();

    std::vector<uint8_t> headerDecoderMemory;
    headerDecoderMemory.resize(mlsdk_decoder_header_decoder_mem_reqs());
    mlsdk_decoder_header_decoder *headerDecoder = mlsdk_decoder_create_header_decoder(
        data.c_str(), static_cast<uint64_t>(data.size()), headerDecoderMemory.data());
    ASSERT_TRUE(mlsdk_decoder_is_header_valid(headerDecoder));
    ASSERT_TRUE(mlsdk_decoder_is_header_compatible(headerDecoder));

    mlsdk_decoder_vgf_section_info section;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_resources, &section);
    ASSERT_TRUE(mlsdk_decoder_is_valid_model_resource_table(data.c_str() + section.offset, section.size));

    std::vector<uint8_t> resourceTableDecoderMemory;
    resourceTableDecoderMemory.resize(mlsdk_decoder_model_resource_table_decoder_mem_reqs());
    mlsdk_decoder_model_resource_table_decoder *resourceTableDecoder =
        mlsdk_decoder_create_model_resource_table_decoder(data.c_str() + section.offset, section.size,
                                                          resourceTableDecoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_model_resource_table_num_entries(resourceTableDecoder) == 2);

    ASSERT_TRUE(mlsdk_decoder_model_resource_table_get_category(resourceTableDecoder, resource0.reference) ==
                mlsdk_decoder_mrt_category_input);
    ASSERT_TRUE(mlsdk_decoder_get_vk_descriptor_type(resourceTableDecoder, resource0.reference).has_value);
    ASSERT_TRUE(mlsdk_decoder_get_vk_descriptor_type(resourceTableDecoder, resource0.reference).value ==
                mlsdk_vk_descriptor_type_storage_image);
    ASSERT_TRUE(mlsdk_decoder_get_vk_format(resourceTableDecoder, resource0.reference) ==
                mlsdk_vk_format_r4g4_unorm_pack8);

    mlsdk_decoder_tensor_dimensions tensorShape1;
    mlsdk_decoder_model_resource_table_get_tensor_shape(resourceTableDecoder, resource0.reference, &tensorShape1);
    ASSERT_TRUE(DataView<int64_t>(tensorShape1.data, tensorShape1.size) ==
                DataView<int64_t>(shape1.data(), shape1.size()));

    ASSERT_TRUE(mlsdk_decoder_model_resource_table_get_category(resourceTableDecoder, resource1.reference) ==
                mlsdk_decoder_mrt_category_constant);
    ASSERT_TRUE(mlsdk_decoder_get_vk_descriptor_type(resourceTableDecoder, resource1.reference).has_value == false);
    ASSERT_TRUE(mlsdk_decoder_get_vk_format(resourceTableDecoder, resource1.reference) ==
                mlsdk_vk_format_r4g4b4a4_unorm_pack16);

    mlsdk_decoder_tensor_dimensions tensorShape2;
    mlsdk_decoder_model_resource_table_get_tensor_shape(resourceTableDecoder, resource1.reference, &tensorShape2);
    ASSERT_TRUE(DataView<int64_t>(tensorShape2.data, tensorShape2.size) ==
                DataView<int64_t>(shape2.data(), shape2.size()));
}
