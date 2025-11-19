/*
 * SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "vgf/decoder.h"
#include "vgf/decoder.hpp"
#include "vgf/encoder.hpp"
#include "vgf/types.hpp"

#include "header.hpp"
#include "vgf-utils/memory_map.hpp"
#include "vgf-utils/temp_folder.hpp"

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
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();
    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(data.c_str());
    ASSERT_TRUE(headerDecoder->IsValid());
    ASSERT_TRUE(headerDecoder->CheckVersion());

    ASSERT_TRUE(VerifyConstant(data.c_str() + headerDecoder->GetConstantsOffset(), headerDecoder->GetConstantsSize()));
    std::unique_ptr<ConstantDecoder> decoder =
        CreateConstantDecoder(data.c_str() + headerDecoder->GetConstantsOffset());

    ASSERT_TRUE(decoder->size() == 1);
    ASSERT_TRUE(decoder->getConstant(constantRef.reference) == DataView<uint8_t>(constant.data(), constant.size()));
    ASSERT_TRUE(decoder->getConstantMrtIndex(constantRef.reference) == resourceRef.reference);
    ASSERT_TRUE(decoder->isSparseConstant(constantRef.reference));
    ASSERT_TRUE(decoder->getConstantSparsityDimension(constantRef.reference) == sparsityDimension);
}

TEST(CppEncodeDecode, AddNonSparseConstant) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);

    ResourceRef resourceRef = {42};
    const std::vector<uint8_t> constant{1};

    ConstantRef constantRef = encoder->AddConstant(resourceRef, constant.data(), constant.size());

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();
    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(data.c_str());
    ASSERT_TRUE(headerDecoder->IsValid());
    ASSERT_TRUE(headerDecoder->CheckVersion());

    ASSERT_TRUE(VerifyConstant(data.c_str() + headerDecoder->GetConstantsOffset(), headerDecoder->GetConstantsSize()));
    std::unique_ptr<ConstantDecoder> decoder =
        CreateConstantDecoder(data.c_str() + headerDecoder->GetConstantsOffset());

    ASSERT_TRUE(decoder->size() == 1);
    ASSERT_TRUE(decoder->getConstant(constantRef.reference) == DataView<uint8_t>(constant.data(), constant.size()));
    ASSERT_TRUE(decoder->getConstantMrtIndex(constantRef.reference) == resourceRef.reference);
    ASSERT_TRUE(decoder->isSparseConstant(constantRef.reference) == false);
}

TEST(CppEncodeDecode, AddManyLargeNonSparseConstant) {
    TempFolder tempFolder("vgf_lib_model");
    const std::string filename = tempFolder.relative("Model.bin").string();
    std::ofstream file(filename, std::ios::binary);
    ASSERT_TRUE(file);

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);

    const size_t largeConstsSize = 25000000; // 25MB
    const std::vector<uint8_t> largeConst(largeConstsSize, 'l');
    const uint32_t numLargeConsts = 40;
    const size_t smallConstsSize = 2000; // 2KB
    const std::vector<uint8_t> smallConst(smallConstsSize, 's');
    const uint32_t numSmallConsts = 1000;
    const size_t veryLargeConstsSize = 500000000; // 500MB
    const std::vector<uint8_t> veryLargeConst(veryLargeConstsSize, 'L');
    const uint32_t numVeryLargeConsts = 10;
    const size_t verySmallConstsSize = 1; // 1B
    const std::vector<uint8_t> verySmallConst(verySmallConstsSize, 'S');
    const uint32_t numVerySmallConsts = 100000;

    std::vector<ConstantRef> constants;
    constants.reserve(numLargeConsts + numSmallConsts + numVeryLargeConsts + numVerySmallConsts);

    uint32_t constantsIndex = 0;
    for (; constantsIndex < numLargeConsts; ++constantsIndex) {
        ResourceRef resourceRef = {constantsIndex};
        constants.push_back(encoder->AddConstant(resourceRef, largeConst.data(), largeConst.size()));
    }

    for (; constantsIndex < numLargeConsts + numSmallConsts; ++constantsIndex) {
        ResourceRef resourceRef = {constantsIndex};
        constants.push_back(encoder->AddConstant(resourceRef, smallConst.data(), smallConst.size()));
    }

    for (; constantsIndex < numLargeConsts + numSmallConsts + numVeryLargeConsts; ++constantsIndex) {
        ResourceRef resourceRef = {constantsIndex};
        constants.push_back(encoder->AddConstant(resourceRef, veryLargeConst.data(), veryLargeConst.size()));
    }

    for (; constantsIndex < numLargeConsts + numSmallConsts + numVeryLargeConsts + numVerySmallConsts;
         ++constantsIndex) {
        ResourceRef resourceRef = {constantsIndex};
        constants.push_back(encoder->AddConstant(resourceRef, verySmallConst.data(), verySmallConst.size()));
    }

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(file)) << "Filename: " << filename;
    file.close();

    auto mmapped = MemoryMap(filename);

    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(mmapped.ptr());
    ASSERT_TRUE(headerDecoder->IsValid());
    ASSERT_TRUE(headerDecoder->CheckVersion());

    ASSERT_TRUE(VerifyConstant(mmapped.ptr(headerDecoder->GetConstantsOffset()), headerDecoder->GetConstantsSize()));

    std::unique_ptr<ConstantDecoder> decoder = CreateConstantDecoder(mmapped.ptr(headerDecoder->GetConstantsOffset()));

    ASSERT_TRUE(decoder->size() == numLargeConsts + numSmallConsts + numVeryLargeConsts + numVerySmallConsts);
    const DataView<uint8_t> largeConstsDataView(largeConst.data(), largeConstsSize);
    for (uint32_t i = 0; i < numLargeConsts; ++i) {
        ASSERT_TRUE(decoder->getConstant(i) == largeConstsDataView);
        ASSERT_TRUE(decoder->getConstantMrtIndex(i) == constants[i].reference);
        ASSERT_TRUE(decoder->isSparseConstant(i) == false);
    }
    const DataView<uint8_t> smallConstsDataView(smallConst.data(), smallConstsSize);
    for (uint32_t i = numLargeConsts; i < numLargeConsts + numSmallConsts; ++i) {
        ASSERT_TRUE(decoder->getConstant(i) == smallConstsDataView);
        ASSERT_TRUE(decoder->getConstantMrtIndex(i) == constants[i].reference);
        ASSERT_TRUE(decoder->isSparseConstant(i) == false);
    }
    const DataView<uint8_t> veryLargeConstsDataView(veryLargeConst.data(), veryLargeConstsSize);
    for (uint32_t i = numLargeConsts + numSmallConsts; i < numLargeConsts + numSmallConsts + numVeryLargeConsts; ++i) {
        ASSERT_TRUE(decoder->getConstant(i) == veryLargeConstsDataView);
        ASSERT_TRUE(decoder->getConstantMrtIndex(i) == constants[i].reference);
        ASSERT_TRUE(decoder->isSparseConstant(i) == false);
    }
    const DataView<uint8_t> verySmallConstsDataView(verySmallConst.data(), verySmallConstsSize);
    for (uint32_t i = numLargeConsts + numSmallConsts + numVeryLargeConsts;
         i < numLargeConsts + numSmallConsts + numVeryLargeConsts + numVerySmallConsts; ++i) {
        ASSERT_TRUE(decoder->getConstant(i) == verySmallConstsDataView);
        ASSERT_TRUE(decoder->getConstantMrtIndex(i) == constants[i].reference);
        ASSERT_TRUE(decoder->isSparseConstant(i) == false);
    }
}

TEST(CppEncodeDecode, EmptyConstantSection) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();
    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(data.c_str());
    ASSERT_TRUE(headerDecoder->IsValid());
    ASSERT_TRUE(headerDecoder->CheckVersion());

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
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();

    ASSERT_TRUE(data.size() >= mlsdk_decoder_header_size());

    std::vector<uint8_t> headerDecoderMemory;
    headerDecoderMemory.resize(mlsdk_decoder_header_decoder_mem_reqs());
    mlsdk_decoder_header_decoder *headerDecoder =
        mlsdk_decoder_create_header_decoder(data.c_str(), headerDecoderMemory.data());
    ASSERT_TRUE(mlsdk_decoder_is_header_valid(headerDecoder));
    ASSERT_TRUE(mlsdk_decoder_is_header_compatible(headerDecoder));

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
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();

    ASSERT_TRUE(data.size() >= mlsdk_decoder_header_size());

    std::vector<uint8_t> headerDecoderMemory;
    headerDecoderMemory.resize(mlsdk_decoder_header_decoder_mem_reqs());
    mlsdk_decoder_header_decoder *headerDecoder =
        mlsdk_decoder_create_header_decoder(data.c_str(), headerDecoderMemory.data());
    ASSERT_TRUE(mlsdk_decoder_is_header_valid(headerDecoder));
    ASSERT_TRUE(mlsdk_decoder_is_header_compatible(headerDecoder));

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

TEST(CEncodeDecode, AddManyLargeNonSparseConstant) {
    TempFolder tempFolder("vgf_lib_model");
    const std::string filename = tempFolder.relative("Model.bin").string();
    std::ofstream file(filename, std::ios::binary);
    ASSERT_TRUE(file);

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);

    const size_t largeConstsSize = 25000000; // 25MB
    const std::vector<uint8_t> largeConst(largeConstsSize, 'l');
    const uint32_t numLargeConsts = 40;
    const size_t smallConstsSize = 2000; // 2KB
    const std::vector<uint8_t> smallConst(smallConstsSize, 's');
    const uint32_t numSmallConsts = 1000;
    const size_t veryLargeConstsSize = 500000000; // 500MB
    const std::vector<uint8_t> veryLargeConst(veryLargeConstsSize, 'L');
    const uint32_t numVeryLargeConsts = 10;
    const size_t verySmallConstsSize = 1; // 1B
    const std::vector<uint8_t> verySmallConst(verySmallConstsSize, 'S');
    const uint32_t numVerySmallConsts = 100000;

    std::vector<ConstantRef> constants;
    constants.reserve(numLargeConsts + numSmallConsts + numVeryLargeConsts + numVerySmallConsts);

    uint32_t constantsIndex = 0;
    for (; constantsIndex < numLargeConsts; ++constantsIndex) {
        ResourceRef resourceRef = {constantsIndex};
        constants.push_back(encoder->AddConstant(resourceRef, largeConst.data(), largeConst.size()));
    }

    for (; constantsIndex < numLargeConsts + numSmallConsts; ++constantsIndex) {
        ResourceRef resourceRef = {constantsIndex};
        constants.push_back(encoder->AddConstant(resourceRef, smallConst.data(), smallConst.size()));
    }

    for (; constantsIndex < numLargeConsts + numSmallConsts + numVeryLargeConsts; ++constantsIndex) {
        ResourceRef resourceRef = {constantsIndex};
        constants.push_back(encoder->AddConstant(resourceRef, veryLargeConst.data(), veryLargeConst.size()));
    }

    for (; constantsIndex < numLargeConsts + numSmallConsts + numVeryLargeConsts + numVerySmallConsts;
         ++constantsIndex) {
        ResourceRef resourceRef = {constantsIndex};
        constants.push_back(encoder->AddConstant(resourceRef, verySmallConst.data(), verySmallConst.size()));
    }

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(file)) << "Filename: " << filename;
    file.close();

    auto mmapped = MemoryMap(filename);
    ASSERT_TRUE(mmapped.size() >= mlsdk_decoder_header_size());

    std::vector<uint8_t> headerDecoderMemory;
    headerDecoderMemory.resize(mlsdk_decoder_header_decoder_mem_reqs());
    mlsdk_decoder_header_decoder *headerDecoder =
        mlsdk_decoder_create_header_decoder(mmapped.ptr(), headerDecoderMemory.data());
    ASSERT_TRUE(mlsdk_decoder_is_header_valid(headerDecoder));
    ASSERT_TRUE(mlsdk_decoder_is_header_compatible(headerDecoder));

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
        mlsdk_decoder_is_valid_constant_table(mmapped.ptr(modelConstantsSection.offset), modelConstantsSection.size));

    std::vector<uint8_t> constantDecoderMemory;
    constantDecoderMemory.resize(mlsdk_decoder_constant_table_decoder_mem_reqs());
    mlsdk_decoder_constant_table_decoder *decoder = mlsdk_decoder_create_constant_table_decoder(
        mmapped.ptr(modelConstantsSection.offset), constantDecoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_constant_table_num_entries(decoder) ==
                numLargeConsts + numSmallConsts + numVeryLargeConsts + numVerySmallConsts);

    mlsdk_decoder_constant_data constantData;
    const DataView<uint8_t> largeConstsDataView(largeConst.data(), largeConstsSize);
    for (uint32_t i = 0; i < numLargeConsts; ++i) {
        mlsdk_decoder_constant_table_get_data(decoder, i, &constantData);
        ASSERT_TRUE(DataView<uint8_t>(constantData.data, constantData.size) == largeConstsDataView);
    }
    const DataView<uint8_t> smallConstsDataView(smallConst.data(), smallConstsSize);
    for (uint32_t i = numLargeConsts; i < numLargeConsts + numSmallConsts; ++i) {
        mlsdk_decoder_constant_table_get_data(decoder, i, &constantData);
        ASSERT_TRUE(DataView<uint8_t>(constantData.data, constantData.size) == smallConstsDataView);
    }
    const DataView<uint8_t> veryLargeConstsDataView(veryLargeConst.data(), veryLargeConstsSize);
    for (uint32_t i = numLargeConsts + numSmallConsts; i < numLargeConsts + numSmallConsts + numVeryLargeConsts; ++i) {
        mlsdk_decoder_constant_table_get_data(decoder, i, &constantData);
        ASSERT_TRUE(DataView<uint8_t>(constantData.data, constantData.size) == veryLargeConstsDataView);
    }
    const DataView<uint8_t> verySmallConstsDataView(verySmallConst.data(), verySmallConstsSize);
    for (uint32_t i = numLargeConsts + numSmallConsts + numVeryLargeConsts;
         i < numLargeConsts + numSmallConsts + numVeryLargeConsts + numVerySmallConsts; ++i) {
        mlsdk_decoder_constant_table_get_data(decoder, i, &constantData);
        ASSERT_TRUE(DataView<uint8_t>(constantData.data, constantData.size) == verySmallConstsDataView);
    }
}

TEST(CEncodeDecode, EmptyConstantSection) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();

    ASSERT_TRUE(data.size() >= mlsdk_decoder_header_size());

    std::vector<uint8_t> headerDecoderMemory;
    headerDecoderMemory.resize(mlsdk_decoder_header_decoder_mem_reqs());
    mlsdk_decoder_header_decoder *headerDecoder =
        mlsdk_decoder_create_header_decoder(data.c_str(), headerDecoderMemory.data());
    ASSERT_TRUE(mlsdk_decoder_is_header_valid(headerDecoder));
    ASSERT_TRUE(mlsdk_decoder_is_header_compatible(headerDecoder));

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
