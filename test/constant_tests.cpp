/*
 * SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "vgf/decoder.h"
#include "vgf/decoder.hpp"
#include "vgf/encoder.hpp"
#include "vgf/types.hpp"

#include "header.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>

using namespace mlsdk::vgflib;

const uint16_t pretendVulkanHeaderVersion = 123;

TEST(CppVerify, BadData) {

    uint8_t bad_data[16] = {0xde, 0xad, 0xbe, 0xef, 0xba, 0xad, 0xf0, 0x0d,
                            0xca, 0xfe, 0xba, 0xbe, 0x00, 0x11, 0x22, 0x33};

    const void *bad_data_ptr = static_cast<const void *>(bad_data);
    ASSERT_FALSE(VerifyConstant(bad_data_ptr, sizeof(bad_data)));
}

TEST(CppEncodeDecode, AddConstant) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);

    ResourceRef resourceRef = {42};
    const std::vector<uint8_t> constant{'a', 'b'};
    int64_t sparsityDimension = 1;

    ConstantRef constantRef = encoder->AddConstant(resourceRef, constant.data(), constant.size(), sparsityDimension);

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer) == true);

    std::string data = buffer.str();
    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(data.c_str());
    ASSERT_TRUE(headerDecoder->IsValid() == true);
    ASSERT_TRUE(headerDecoder->CheckVersion() == true);

    ASSERT_TRUE(VerifyConstant(data.c_str() + headerDecoder->GetConstantsOffset(), headerDecoder->GetConstantsSize()));
    std::unique_ptr<ConstantDecoder> decoder =
        CreateConstantDecoder(data.c_str() + headerDecoder->GetConstantsOffset());

    ASSERT_TRUE(decoder->size() == 1);
    ASSERT_TRUE(decoder->getConstant(constantRef.reference) == DataView<uint8_t>(constant.data(), constant.size()));
    ASSERT_TRUE(decoder->getConstantMrtIndex(constantRef.reference) == resourceRef.reference);
    ASSERT_TRUE(decoder->isSparseConstant(constantRef.reference) == true);
    ASSERT_TRUE(decoder->getConstantSparsityDimension(constantRef.reference) == sparsityDimension);
}

TEST(CppEncodeDecode, AddNonSparseConstant) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);

    ResourceRef resourceRef = {42};
    const std::vector<uint8_t> constant{1};

    ConstantRef constantRef = encoder->AddConstant(resourceRef, constant.data(), constant.size());

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer) == true);

    std::string data = buffer.str();
    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(data.c_str());
    ASSERT_TRUE(headerDecoder->IsValid() == true);
    ASSERT_TRUE(headerDecoder->CheckVersion() == true);

    ASSERT_TRUE(VerifyConstant(data.c_str() + headerDecoder->GetConstantsOffset(), headerDecoder->GetConstantsSize()));
    std::unique_ptr<ConstantDecoder> decoder =
        CreateConstantDecoder(data.c_str() + headerDecoder->GetConstantsOffset());

    ASSERT_TRUE(decoder->size() == 1);
    ASSERT_TRUE(decoder->getConstant(constantRef.reference) == DataView<uint8_t>(constant.data(), constant.size()));
    ASSERT_TRUE(decoder->getConstantMrtIndex(constantRef.reference) == resourceRef.reference);
    ASSERT_TRUE(decoder->isSparseConstant(constantRef.reference) == false);
}

TEST(CppEncodeDecode, EmptyConstantSection) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer) == true);

    std::string data = buffer.str();
    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(data.c_str());
    ASSERT_TRUE(headerDecoder->IsValid() == true);
    ASSERT_TRUE(headerDecoder->CheckVersion() == true);

    ASSERT_TRUE(VerifyConstant(data.c_str() + headerDecoder->GetConstantsOffset(), headerDecoder->GetConstantsSize()));
    std::unique_ptr<ConstantDecoder> decoder =
        CreateConstantDecoder(data.c_str() + headerDecoder->GetConstantsOffset());

    ASSERT_TRUE(decoder->size() == 0);
}

TEST(CEncodeDecode, AddConstant) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);

    ResourceRef resourceRef = {42};
    const std::vector<uint8_t> constant{'a', 'b'};
    int64_t sparsityDimension = 1;

    ConstantRef constantRef = encoder->AddConstant(resourceRef, constant.data(), constant.size(), sparsityDimension);

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer) == true);

    std::string data = buffer.str();

    ASSERT_TRUE(data.size() >= mlsdk_decoder_header_size());

    std::vector<uint8_t> headerDecoderMemory;
    headerDecoderMemory.resize(mlsdk_decoder_header_decoder_mem_reqs());
    mlsdk_decoder_header_decoder *headerDecoder =
        mlsdk_decoder_create_header_decoder(data.c_str(), headerDecoderMemory.data());
    ASSERT_TRUE(mlsdk_decoder_is_header_valid(headerDecoder) == true);
    ASSERT_TRUE(mlsdk_decoder_is_header_compatible(headerDecoder) == true);

    mlsdk_decoder_vgf_section_info moduleSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_modules, &moduleSection);
    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    mlsdk_decoder_vgf_section_info modelResourceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_resources, &modelResourceSection);
    mlsdk_decoder_vgf_section_info modelConstantsSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_constants, &modelConstantsSection);
    ASSERT_TRUE(modelConstantsSection.size > 0);
    ASSERT_TRUE(modelConstantsSection.offset ==
                HEADER_HEADER_SIZE_VALUE + moduleSection.size + modelSequenceSection.size + modelResourceSection.size);
    ASSERT_TRUE(
        mlsdk_decoder_is_valid_constant_table(data.c_str() + modelConstantsSection.offset, modelConstantsSection.size));

    std::vector<uint8_t> constantDecoderMemory;
    constantDecoderMemory.resize(mlsdk_decoder_constant_table_decoder_mem_reqs());
    mlsdk_decoder_constant_table_decoder *decoder = mlsdk_decoder_create_constant_table_decoder(
        data.c_str() + modelConstantsSection.offset, constantDecoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_constant_table_num_entries(decoder) == 1);

    mlsdk_decoder_constant_data constantData;
    mlsdk_decoder_constant_table_get_data(decoder, constantRef.reference, &constantData);

    ASSERT_TRUE(42 == mlsdk_decoder_constant_table_get_mrt_index(decoder, constantRef.reference));
    ASSERT_TRUE(true == mlsdk_decoder_constant_table_is_sparse(decoder, constantRef.reference));
    ASSERT_TRUE(1 == mlsdk_decoder_constant_table_get_sparsity_dimension(decoder, constantRef.reference));
    ASSERT_TRUE(DataView<uint8_t>(constantData.data, constantData.size) ==
                DataView<uint8_t>(constant.data(), constant.size()));
}

TEST(CEncodeDecode, AddNonSparseConstant) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);

    ResourceRef resourceRef = {42};
    const std::vector<uint8_t> constant{1};

    ConstantRef constantRef = encoder->AddConstant(resourceRef, constant.data(), constant.size());

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer) == true);

    std::string data = buffer.str();

    ASSERT_TRUE(data.size() >= mlsdk_decoder_header_size());

    std::vector<uint8_t> headerDecoderMemory;
    headerDecoderMemory.resize(mlsdk_decoder_header_decoder_mem_reqs());
    mlsdk_decoder_header_decoder *headerDecoder =
        mlsdk_decoder_create_header_decoder(data.c_str(), headerDecoderMemory.data());
    ASSERT_TRUE(mlsdk_decoder_is_header_valid(headerDecoder) == true);
    ASSERT_TRUE(mlsdk_decoder_is_header_compatible(headerDecoder) == true);

    mlsdk_decoder_vgf_section_info moduleSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_modules, &moduleSection);
    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    mlsdk_decoder_vgf_section_info modelResourceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_resources, &modelResourceSection);
    mlsdk_decoder_vgf_section_info modelConstantsSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_constants, &modelConstantsSection);
    ASSERT_TRUE(modelConstantsSection.size > 0);
    ASSERT_TRUE(modelConstantsSection.offset ==
                HEADER_HEADER_SIZE_VALUE + moduleSection.size + modelSequenceSection.size + modelResourceSection.size);
    ASSERT_TRUE(
        mlsdk_decoder_is_valid_constant_table(data.c_str() + modelConstantsSection.offset, modelConstantsSection.size));

    std::vector<uint8_t> constantDecoderMemory;
    constantDecoderMemory.resize(mlsdk_decoder_constant_table_decoder_mem_reqs());
    mlsdk_decoder_constant_table_decoder *decoder = mlsdk_decoder_create_constant_table_decoder(
        data.c_str() + modelConstantsSection.offset, constantDecoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_constant_table_num_entries(decoder) == 1);

    mlsdk_decoder_constant_data constantData;
    mlsdk_decoder_constant_table_get_data(decoder, constantRef.reference, &constantData);

    ASSERT_TRUE(42 == mlsdk_decoder_constant_table_get_mrt_index(decoder, constantRef.reference));
    ASSERT_TRUE(false == mlsdk_decoder_constant_table_is_sparse(decoder, constantRef.reference));
    ASSERT_TRUE(DataView<uint8_t>(constantData.data, constantData.size) ==
                DataView<uint8_t>(constant.data(), constant.size()));
}

TEST(CEncodeDecode, EmptyConstantSection) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer) == true);

    std::string data = buffer.str();

    ASSERT_TRUE(data.size() >= mlsdk_decoder_header_size());

    std::vector<uint8_t> headerDecoderMemory;
    headerDecoderMemory.resize(mlsdk_decoder_header_decoder_mem_reqs());
    mlsdk_decoder_header_decoder *headerDecoder =
        mlsdk_decoder_create_header_decoder(data.c_str(), headerDecoderMemory.data());
    ASSERT_TRUE(mlsdk_decoder_is_header_valid(headerDecoder) == true);
    ASSERT_TRUE(mlsdk_decoder_is_header_compatible(headerDecoder) == true);

    mlsdk_decoder_vgf_section_info moduleSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_modules, &moduleSection);
    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    mlsdk_decoder_vgf_section_info modelResourceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_resources, &modelResourceSection);
    mlsdk_decoder_vgf_section_info modelConstantsSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_constants, &modelConstantsSection);
    ASSERT_TRUE(modelConstantsSection.size > 0);
    ASSERT_TRUE(modelConstantsSection.offset ==
                HEADER_HEADER_SIZE_VALUE + moduleSection.size + modelSequenceSection.size + modelResourceSection.size);
    ASSERT_TRUE(
        mlsdk_decoder_is_valid_constant_table(data.c_str() + modelConstantsSection.offset, modelConstantsSection.size));

    std::vector<uint8_t> constantDecoderMemory;
    constantDecoderMemory.resize(mlsdk_decoder_constant_table_decoder_mem_reqs());
    mlsdk_decoder_constant_table_decoder *decoder = mlsdk_decoder_create_constant_table_decoder(
        data.c_str() + modelConstantsSection.offset, constantDecoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_constant_table_num_entries(decoder) == 0);
}

// TODO: different datatypes.
