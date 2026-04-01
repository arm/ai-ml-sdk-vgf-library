/*
 * SPDX-FileCopyrightText: Copyright 2023-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common.hpp"
#include "vgf/decoder.h"
#include "vgf/decoder.hpp"
#include "vgf/encoder.hpp"
#include "vgf/logging.hpp"
#include "vgf/types.hpp"

#include "header.hpp"

#include <gtest/gtest.h>

#include <array>
#include <sstream>
#include <string>
#include <vector>

using namespace mlsdk::vgflib;
using logging::utils::Logger;

namespace {
const uint16_t pretendVulkanHeaderVersion = 123;

constexpr DescriptorType VK_DESCRIPTOR_TYPE_STORAGE_IMAGE = 3;
constexpr FormatType VK_FORMAT_R4G4_UNORM_PACK8 = 1;
constexpr FormatType VK_FORMAT_R4G4B4A4_UNORM_PACK16 = 12;

constexpr bool DataViewTests() {
    static_assert(DataView<uint8_t>().empty(), "Default constructor should create an empty view");
    // NOLINTNEXTLINE code coverage
    static_assert(DataView<uint8_t>().size() == 0, "Default constructor should create a view of size 0");
    static_assert(DataView<uint8_t>(nullptr, 0).empty(),
                  "Constructor with nullptr and size 0 should create an empty view");
    // NOLINTNEXTLINE code coverage
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
        CreateHeaderDecoder(data.c_str(), static_cast<uint64_t>(HeaderSize()), static_cast<uint64_t>(data.size()));
    ASSERT_NE(headerDecoder, nullptr);

    std::unique_ptr<ModelResourceTableDecoder> decoder = CreateModelResourceTableDecoder(
        data.c_str() + headerDecoder->GetModelResourceTableOffset(), headerDecoder->GetModelResourceTableSize());
    ASSERT_NE(decoder, nullptr);

    ASSERT_TRUE(decoder->size() == 0);
}

TEST(CppModelResourceTable, EncodeDecode) {
    std::stringstream buffer;

    //! [MrtEncodeInput begin]
    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    std::vector<int64_t> shape1{0, 1, 2, 3};
    std::vector<int64_t> strides1{4, 5, 6, 7};

    ResourceRef resource0 =
        encoder->AddInputResource(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R4G4_UNORM_PACK8, shape1, strides1);
    ResourceRef resource1 =
        encoder->AddOutputResource(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R4G4_UNORM_PACK8, shape1, strides1);
    //! [MrtEncodeInput end]
    (void)resource1;

    //! [MrtEncodeConstant begin]
    std::vector<int64_t> shape2{8, 9, 10, 11};
    std::vector<int64_t> strides2{12, 13, 14, 15};
    ResourceRef resource2 = encoder->AddConstantResource(VK_FORMAT_R4G4B4A4_UNORM_PACK16, shape2, strides2);
    //! [MrtEncodeConstant end]

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string vgfData = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(
        vgfData.c_str(), static_cast<uint64_t>(HeaderSize()), static_cast<uint64_t>(vgfData.size()));
    ASSERT_NE(headerDecoder, nullptr);

    uint32_t mrtIndex = resource0.reference;
    //! [MrtDecodingSample0 begin]
    std::unique_ptr<ModelResourceTableDecoder> mrtDecoder = CreateModelResourceTableDecoder(
        vgfData.c_str() + headerDecoder->GetModelResourceTableOffset(), headerDecoder->GetModelResourceTableSize());
    ASSERT_NE(mrtDecoder, nullptr);

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

    auto resource0 =
        encoder->AddInputResource(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R4G4_UNORM_PACK8, shape1, {});
    [[maybe_unused]] auto resource1 = encoder->AddConstantResource(VK_FORMAT_R4G4B4A4_UNORM_PACK16, shape2, {});
    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string vgfData = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(
        vgfData.c_str(), static_cast<uint64_t>(HeaderSize()), static_cast<uint64_t>(vgfData.size()));
    ASSERT_NE(headerDecoder, nullptr);

    uint32_t mrtIndex = resource0.reference;
    std::unique_ptr<ModelResourceTableDecoder> mrtDecoder = CreateModelResourceTableDecoder(
        vgfData.c_str() + headerDecoder->GetModelResourceTableOffset(), headerDecoder->GetModelResourceTableSize());
    ASSERT_NE(mrtDecoder, nullptr);

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

TEST(CppVerify, ModelResourceSizeWrapRejected) {
    Logger logger;
    const uint64_t resourceOffset = 46;
    const auto resourceSize = static_cast<uint64_t>(SIZE_MAX_VALUE);
    const size_t fileSize = 296;

    std::vector<uint8_t> buffer(fileSize, 0);
    Header header({0, 0}, {0, 0}, {resourceOffset, resourceSize}, {0, 0}, pretendVulkanHeaderVersion);
    std::memcpy(buffer.data(), &header, sizeof(Header));

    EXPECT_EQ(nullptr, CreateHeaderDecoder(buffer.data(), static_cast<uint64_t>(HeaderSize()),
                                           static_cast<uint64_t>(buffer.size())));
    EXPECT_TRUE(logger.contains({"section bounds invalid"}));
    EXPECT_EQ(nullptr, CreateModelResourceTableDecoder(buffer.data() + resourceOffset, resourceSize));
    EXPECT_TRUE(logger.contains({"VerifyModelResourceTable", "size out of bounds"}));
}

TEST(CppVerify, ModelResourceTooSmallRejected) {
    Logger logger;
    std::array<uint8_t, 2> buffer{0, 0};

    EXPECT_EQ(nullptr, CreateModelResourceTableDecoder(buffer.data(), buffer.size()));
    EXPECT_TRUE(logger.contains({"VerifyModelResourceTable", "size smaller than header"}));
}

TEST(CppVerify, ModelResourceMisalignedRejected) {
    Logger logger;
    const uint64_t resourceOffset = 129;
    const uint64_t resourceSize = 32;
    const size_t fileSize = 177;

    std::vector<uint8_t> buffer(fileSize, 0);
    Header header({0, 0}, {0, 0}, {resourceOffset, resourceSize}, {0, 0}, pretendVulkanHeaderVersion);
    std::memcpy(buffer.data(), &header, sizeof(Header));

    EXPECT_NE(nullptr, CreateHeaderDecoder(buffer.data(), static_cast<uint64_t>(HeaderSize()),
                                           static_cast<uint64_t>(buffer.size())));
    EXPECT_EQ(nullptr, CreateModelResourceTableDecoder(buffer.data() + resourceOffset, resourceSize));
    EXPECT_TRUE(logger.contains({"VerifyModelResourceTable", "data alignment invalid"}));
}

TEST(CppVerify, ModelResourceFlatbufferVerifyRejected) {
    Logger logger;
    std::array<uint8_t, 32> buffer{};
    buffer.fill(0xFF);

    EXPECT_EQ(nullptr, CreateModelResourceTableDecoder(buffer.data(), buffer.size()));
    EXPECT_TRUE(logger.contains({"VerifyModelResourceTable", "verification failed"}));
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
    mlsdk_decoder_header_decoder *headerDecoder =
        mlsdk_decoder_create_header_decoder(data.c_str(), static_cast<uint64_t>(mlsdk_decoder_header_size()),
                                            static_cast<uint64_t>(data.size()), headerDecoderMemory.data());
    ASSERT_TRUE(mlsdk_decoder_is_header_valid(headerDecoder));
    ASSERT_TRUE(mlsdk_decoder_is_header_compatible(headerDecoder));

    mlsdk_decoder_vgf_section_info section;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_resources, &section);

    std::vector<uint8_t> resourceTableMemory;
    resourceTableMemory.resize(mlsdk_decoder_model_resource_table_decoder_mem_reqs());
    mlsdk_decoder_model_resource_table_decoder *decoder = mlsdk_decoder_create_model_resource_table_decoder(
        data.c_str() + section.offset, section.size, resourceTableMemory.data());
    ASSERT_NE(decoder, nullptr);

    ASSERT_TRUE(mlsdk_decoder_get_model_resource_table_num_entries(decoder) == 0);
}

TEST(CModelResourceTable, EncodeDecode) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    std::vector<int64_t> shape1{0, 1, 2, 3};
    std::vector<int64_t> strides1{4, 5, 6, 7};
    std::vector<int64_t> shape2{8, 9, 10, 11};
    std::vector<int64_t> strides2{12, 13, 14, 15};

    mlsdk_vk_descriptor_type mlsdkVkDescriptorTypeStorageImage = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    mlsdk_vk_format mlsdkVkFormatR4g4UnormPack8 = VK_FORMAT_R4G4_UNORM_PACK8;
    mlsdk_vk_format mlsdkVkFormatR4g4b4a4UnormPack16 = VK_FORMAT_R4G4B4A4_UNORM_PACK16;

    auto resource0 =
        encoder->AddInputResource(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R4G4_UNORM_PACK8, shape1, strides1);
    auto resource1 = encoder->AddConstantResource(VK_FORMAT_R4G4B4A4_UNORM_PACK16, shape2, strides2);
    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();

    std::vector<uint8_t> headerDecoderMemory;
    headerDecoderMemory.resize(mlsdk_decoder_header_decoder_mem_reqs());
    mlsdk_decoder_header_decoder *headerDecoder =
        mlsdk_decoder_create_header_decoder(data.c_str(), static_cast<uint64_t>(mlsdk_decoder_header_size()),
                                            static_cast<uint64_t>(data.size()), headerDecoderMemory.data());
    ASSERT_TRUE(mlsdk_decoder_is_header_valid(headerDecoder));
    ASSERT_TRUE(mlsdk_decoder_is_header_compatible(headerDecoder));

    mlsdk_decoder_vgf_section_info section;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_resources, &section);

    std::vector<uint8_t> resourceTableDecoderMemory;
    resourceTableDecoderMemory.resize(mlsdk_decoder_model_resource_table_decoder_mem_reqs());
    mlsdk_decoder_model_resource_table_decoder *resourceTableDecoder =
        mlsdk_decoder_create_model_resource_table_decoder(data.c_str() + section.offset, section.size,
                                                          resourceTableDecoderMemory.data());
    ASSERT_NE(resourceTableDecoder, nullptr);

    ASSERT_TRUE(mlsdk_decoder_get_model_resource_table_num_entries(resourceTableDecoder) == 2);

    ASSERT_TRUE(mlsdk_decoder_model_resource_table_get_category(resourceTableDecoder, resource0.reference) ==
                mlsdk_decoder_mrt_category_input);
    ASSERT_TRUE(mlsdk_decoder_get_vk_descriptor_type(resourceTableDecoder, resource0.reference).has_value);
    ASSERT_TRUE(mlsdk_decoder_get_vk_descriptor_type(resourceTableDecoder, resource0.reference).value ==
                mlsdkVkDescriptorTypeStorageImage);
    ASSERT_TRUE(mlsdk_decoder_get_vk_format(resourceTableDecoder, resource0.reference) == mlsdkVkFormatR4g4UnormPack8);

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
                mlsdkVkFormatR4g4b4a4UnormPack16);

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

    mlsdk_vk_descriptor_type mlsdkVkDescriptorTypeStorageImage = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    mlsdk_vk_format mlsdkVkFormatR4g4UnormPack8 = VK_FORMAT_R4G4_UNORM_PACK8;
    mlsdk_vk_format mlsdkVkFormatR4g4b4a4UnormPack16 = VK_FORMAT_R4G4B4A4_UNORM_PACK16;

    auto resource0 =
        encoder->AddInputResource(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R4G4_UNORM_PACK8, shape1, {});
    auto resource1 = encoder->AddConstantResource(VK_FORMAT_R4G4B4A4_UNORM_PACK16, shape2, {});
    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();

    std::vector<uint8_t> headerDecoderMemory;
    headerDecoderMemory.resize(mlsdk_decoder_header_decoder_mem_reqs());
    mlsdk_decoder_header_decoder *headerDecoder =
        mlsdk_decoder_create_header_decoder(data.c_str(), static_cast<uint64_t>(mlsdk_decoder_header_size()),
                                            static_cast<uint64_t>(data.size()), headerDecoderMemory.data());
    ASSERT_TRUE(mlsdk_decoder_is_header_valid(headerDecoder));
    ASSERT_TRUE(mlsdk_decoder_is_header_compatible(headerDecoder));

    mlsdk_decoder_vgf_section_info section;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_resources, &section);

    std::vector<uint8_t> resourceTableDecoderMemory;
    resourceTableDecoderMemory.resize(mlsdk_decoder_model_resource_table_decoder_mem_reqs());
    mlsdk_decoder_model_resource_table_decoder *resourceTableDecoder =
        mlsdk_decoder_create_model_resource_table_decoder(data.c_str() + section.offset, section.size,
                                                          resourceTableDecoderMemory.data());
    ASSERT_NE(resourceTableDecoder, nullptr);

    ASSERT_TRUE(mlsdk_decoder_get_model_resource_table_num_entries(resourceTableDecoder) == 2);

    ASSERT_TRUE(mlsdk_decoder_model_resource_table_get_category(resourceTableDecoder, resource0.reference) ==
                mlsdk_decoder_mrt_category_input);
    ASSERT_TRUE(mlsdk_decoder_get_vk_descriptor_type(resourceTableDecoder, resource0.reference).has_value);
    ASSERT_TRUE(mlsdk_decoder_get_vk_descriptor_type(resourceTableDecoder, resource0.reference).value ==
                mlsdkVkDescriptorTypeStorageImage);
    ASSERT_TRUE(mlsdk_decoder_get_vk_format(resourceTableDecoder, resource0.reference) == mlsdkVkFormatR4g4UnormPack8);

    mlsdk_decoder_tensor_dimensions tensorShape1;
    mlsdk_decoder_model_resource_table_get_tensor_shape(resourceTableDecoder, resource0.reference, &tensorShape1);
    ASSERT_TRUE(DataView<int64_t>(tensorShape1.data, tensorShape1.size) ==
                DataView<int64_t>(shape1.data(), shape1.size()));

    ASSERT_TRUE(mlsdk_decoder_model_resource_table_get_category(resourceTableDecoder, resource1.reference) ==
                mlsdk_decoder_mrt_category_constant);
    ASSERT_TRUE(mlsdk_decoder_get_vk_descriptor_type(resourceTableDecoder, resource1.reference).has_value == false);
    ASSERT_TRUE(mlsdk_decoder_get_vk_format(resourceTableDecoder, resource1.reference) ==
                mlsdkVkFormatR4g4b4a4UnormPack16);

    mlsdk_decoder_tensor_dimensions tensorShape2;
    mlsdk_decoder_model_resource_table_get_tensor_shape(resourceTableDecoder, resource1.reference, &tensorShape2);
    ASSERT_TRUE(DataView<int64_t>(tensorShape2.data, tensorShape2.size) ==
                DataView<int64_t>(shape2.data(), shape2.size()));
}

TEST(CVerify, ModelResourceSizeWrapRejected) {
    Logger logger;
    const uint64_t resourceOffset = 46;
    const auto resourceSize = static_cast<uint64_t>(SIZE_MAX_VALUE);
    const size_t fileSize = 296;

    std::vector<uint8_t> buffer(fileSize, 0);
    Header header({0, 0}, {0, 0}, {resourceOffset, resourceSize}, {0, 0}, pretendVulkanHeaderVersion);
    std::memcpy(buffer.data(), &header, sizeof(Header));

    std::vector<uint8_t> headerDecoderMemory(mlsdk_decoder_header_decoder_mem_reqs());
    EXPECT_EQ(nullptr,
              mlsdk_decoder_create_header_decoder(buffer.data(), static_cast<uint64_t>(mlsdk_decoder_header_size()),
                                                  static_cast<uint64_t>(buffer.size()), headerDecoderMemory.data()));
    EXPECT_TRUE(logger.contains({"section bounds invalid"}));
    std::vector<uint8_t> decoderMemory(mlsdk_decoder_model_resource_table_decoder_mem_reqs());
    EXPECT_EQ(nullptr, mlsdk_decoder_create_model_resource_table_decoder(buffer.data() + resourceOffset, resourceSize,
                                                                         decoderMemory.data()));
    EXPECT_TRUE(logger.contains({"VerifyModelResourceTable", "size out of bounds"}));
}

TEST(CVerify, ModelResourceTooSmallRejected) {
    Logger logger;
    std::array<uint8_t, 2> buffer{0, 0};

    std::vector<uint8_t> decoderMemory(mlsdk_decoder_model_resource_table_decoder_mem_reqs());
    EXPECT_EQ(nullptr,
              mlsdk_decoder_create_model_resource_table_decoder(buffer.data(), buffer.size(), decoderMemory.data()));
    EXPECT_TRUE(logger.contains({"VerifyModelResourceTable", "size smaller than header"}));
}

TEST(CVerify, ModelResourceMisalignedRejected) {
    Logger logger;
    const uint64_t resourceOffset = 129;
    const uint64_t resourceSize = 32;
    const size_t fileSize = 177;

    std::vector<uint8_t> buffer(fileSize, 0);
    Header header({0, 0}, {0, 0}, {resourceOffset, resourceSize}, {0, 0}, pretendVulkanHeaderVersion);
    std::memcpy(buffer.data(), &header, sizeof(Header));

    std::vector<uint8_t> headerDecoderMemory(mlsdk_decoder_header_decoder_mem_reqs());
    EXPECT_NE(nullptr,
              mlsdk_decoder_create_header_decoder(buffer.data(), static_cast<uint64_t>(mlsdk_decoder_header_size()),
                                                  static_cast<uint64_t>(buffer.size()), headerDecoderMemory.data()));
    std::vector<uint8_t> decoderMem(mlsdk_decoder_model_resource_table_decoder_mem_reqs());
    EXPECT_EQ(nullptr, mlsdk_decoder_create_model_resource_table_decoder(buffer.data() + resourceOffset, resourceSize,
                                                                         decoderMem.data()));
    EXPECT_TRUE(logger.contains({"VerifyModelResourceTable", "data alignment invalid"}));
}

TEST(CVerify, ModelResourceFlatbufferVerifyRejected) {
    Logger logger;
    std::array<uint8_t, 32> buffer{};
    buffer.fill(0xFF);

    std::vector<uint8_t> decoderMemory(mlsdk_decoder_model_resource_table_decoder_mem_reqs());
    EXPECT_EQ(nullptr,
              mlsdk_decoder_create_model_resource_table_decoder(buffer.data(), buffer.size(), decoderMemory.data()));
    EXPECT_TRUE(logger.contains({"VerifyModelResourceTable", "verification failed"}));
}
