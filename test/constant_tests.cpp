/*
 * SPDX-FileCopyrightText: Copyright 2023-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "common.hpp"
#include "vgf/decoder.h"
#include "vgf/decoder.hpp"
#include "vgf/encoder.hpp"
#include "vgf/types.hpp"

#include "constant.hpp"
#include "header.hpp"
#include "vgf-utils/memory_map.hpp"
#include "vgf-utils/temp_folder.hpp"

#include <gtest/gtest.h>

#include <array>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace mlsdk::vgflib;
using logging::utils::Logger;

const uint16_t pretendVulkanHeaderVersion = 123;
constexpr size_t GB{1024 * 1024 * 1024};

namespace {

std::vector<uint8_t> MakeConstantSectionV00(uint64_t count, const std::vector<ConstantMetaDataV00> &metadata,
                                            const std::vector<uint8_t> &constant) {
    const size_t metadataBytes = metadata.size() * sizeof(ConstantMetaDataV00);
    std::vector<uint8_t> buffer(CONSTANT_SECTION_METADATA_OFFSET + metadataBytes + constant.size(), 0);

    std::memcpy(buffer.data() + CONSTANT_SECTION_VERSION_OFFSET, CONSTANT_SECTION_VERSION,
                CONSTANT_SECTION_VERSION_SIZE);
    std::memcpy(buffer.data() + CONSTANT_SECTION_COUNT_OFFSET, &count, sizeof(count));
    if (!metadata.empty()) {
        std::memcpy(buffer.data() + CONSTANT_SECTION_METADATA_OFFSET, metadata.data(), metadataBytes);
    }
    if (!constant.empty()) {
        std::memcpy(buffer.data() + CONSTANT_SECTION_METADATA_OFFSET + metadataBytes, constant.data(), constant.size());
    }
    return buffer;
}

} // namespace

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
    std::unique_ptr<HeaderDecoder> headerDecoder =
        CreateHeaderDecoder(data.c_str(), static_cast<uint64_t>(HeaderSize()), static_cast<uint64_t>(data.size()));
    ASSERT_NE(headerDecoder, nullptr);

    std::unique_ptr<ConstantDecoder> decoder =
        CreateConstantDecoder(data.c_str() + headerDecoder->GetConstantsOffset(), headerDecoder->GetConstantsSize());
    ASSERT_NE(decoder, nullptr);

    ASSERT_TRUE(decoder->size() == 1);
    ASSERT_TRUE(decoder->getConstant(constantRef.reference) == DataView<uint8_t>(constant.data(), constant.size()));
    ASSERT_TRUE(decoder->getConstantMrtIndex(constantRef.reference) == resourceRef.reference);
    ASSERT_TRUE(decoder->isSparseConstant(constantRef.reference));
    ASSERT_NE(decoder->getConstantSparsityDimension(constantRef.reference), CONSTANT_INVALID_SPARSITY_DIMENSION);
    ASSERT_EQ(decoder->getConstantSparsityDimension(constantRef.reference), sparsityDimension);
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
    std::unique_ptr<HeaderDecoder> headerDecoder =
        CreateHeaderDecoder(data.c_str(), static_cast<uint64_t>(HeaderSize()), static_cast<uint64_t>(data.size()));
    ASSERT_NE(headerDecoder, nullptr);

    std::unique_ptr<ConstantDecoder> decoder =
        CreateConstantDecoder(data.c_str() + headerDecoder->GetConstantsOffset(), headerDecoder->GetConstantsSize());
    ASSERT_NE(decoder, nullptr);

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
    const uint32_t numLargeConsts = 10;
    const size_t smallConstsSize = 2000; // 2KB
    const std::vector<uint8_t> smallConst(smallConstsSize, 's');
    const uint32_t numSmallConsts = 1000;
    const size_t veryLargeConstsSize = 500000000; // 500MB
    const std::vector<uint8_t> veryLargeConst(veryLargeConstsSize, 'L');
    const uint32_t numVeryLargeConsts = 4;
    const size_t verySmallConstsSize = 1; // 1B
    const std::vector<uint8_t> verySmallConst(verySmallConstsSize, 'S');
    const uint32_t numVerySmallConsts = 10000;

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

    std::unique_ptr<HeaderDecoder> headerDecoder =
        CreateHeaderDecoder(mmapped.ptr(), static_cast<uint64_t>(HeaderSize()), static_cast<uint64_t>(mmapped.size()));
    ASSERT_NE(headerDecoder, nullptr);

    EXPECT_GT(mmapped.size(), 2 * GB);
    std::unique_ptr<ConstantDecoder> decoder =
        CreateConstantDecoder(mmapped.ptr(headerDecoder->GetConstantsOffset()), headerDecoder->GetConstantsSize());
    ASSERT_NE(decoder, nullptr);

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
TEST(CppVerify, BadData) {

    uint8_t badData[16] = {0xde, 0xad, 0xbe, 0xef, 0xba, 0xad, 0xf0, 0x0d,
                           0xca, 0xfe, 0xba, 0xbe, 0x00, 0x11, 0x22, 0x33};

    const void *badDataPtr = static_cast<const void *>(badData);
    ASSERT_EQ(CreateConstantDecoder(badDataPtr, sizeof(badData)), nullptr);
}

TEST(CppVerify, ConstantHeaderTooSmallRejected) {
    Logger logger;
    std::array<uint8_t, 2> buffer{0, 0};

    EXPECT_EQ(nullptr, CreateConstantDecoder(buffer.data(), buffer.size()));
    EXPECT_TRUE(logger.contains({"Constant section too small to contain version"}));
}

TEST(CppVerify, ConstantSizeCapRejected) {
#if SIZE_MAX < UINT64_MAX
    Logger logger;
    const uint64_t constantSize = SIZE_MAX_VALUE;
    std::array<uint8_t, CONSTANT_SECTION_VERSION_SIZE> section{};
    std::memcpy(section.data(), CONSTANT_SECTION_VERSION, CONSTANT_SECTION_VERSION_SIZE);

    EXPECT_EQ(nullptr, CreateConstantDecoder(section.data(), constantSize));
    EXPECT_TRUE(logger.contains({"VerifyConstant", "Size out of bounds"}));
#else
    GTEST_SKIP() << "Size cap guard only compiled when size_t is narrower than uint64_t";
#endif
}

TEST(CppVerify, SectionTooSmallForMetadataRejected) {
    Logger logger;
    std::vector<uint8_t> section = MakeConstantSectionV00(1, {ConstantMetaDataV00{}}, {});
    section.resize(CONSTANT_SECTION_METADATA_OFFSET - 1);
    ASSERT_EQ(CreateConstantDecoder(section.data(), section.size()), nullptr);
    EXPECT_TRUE(logger.contains({"VerifyConstant", "Constant section too small to contain metadata"}));
}

TEST(CppVerify, ConstantDataOffsetOverflowRejected) {
    Logger logger;
    std::array<uint8_t, CONSTANT_SECTION_METADATA_OFFSET> buffer{};
    std::memcpy(buffer.data(), CONSTANT_SECTION_VERSION, CONSTANT_SECTION_VERSION_SIZE);
    const uint64_t declaredCount = UINT64_MAX;
    std::memcpy(buffer.data() + CONSTANT_SECTION_COUNT_OFFSET, &declaredCount, sizeof(declaredCount));

    EXPECT_EQ(nullptr, CreateConstantDecoder(buffer.data(), static_cast<uint64_t>(buffer.size())));
    EXPECT_TRUE(logger.contains({"VerifyConstant", "Constant section declares more entries than fit in the buffer"}));
}

TEST(CppVerify, DeclaredCountExceedingAvailableMetadataRejected) {
    Logger logger;
    const std::vector<ConstantMetaDataV00> vecMetaData = {
        ConstantMetaDataV00{
            7,  // mrtIndex
            -1, // sparsityDimension
            1,  // size
            0,  // offset
        },
    };
    const std::vector<uint8_t> constant = {'a'};
    const uint64_t declaredCount = 2;
    std::vector<uint8_t> section = MakeConstantSectionV00(declaredCount, vecMetaData, constant);
    ASSERT_EQ(CreateConstantDecoder(section.data(), section.size()), nullptr);
    EXPECT_TRUE(logger.contains({"VerifyConstant", "Constant section declares more entries than fit in the buffer"}));
}

TEST(CppVerify, OutOfRangeOffsetsRejected) {
    Logger logger;
    const std::vector<ConstantMetaDataV00> vecMetadata = {
        ConstantMetaDataV00{
            3,  // mrtIndex
            -1, // sparsityDimension
            10, // size
            5,  // offset
        },
    };
    const std::vector<uint8_t> constant = {'a', 'b', 'c', 'd', 'e'};
    const uint64_t declaredCount = 1;
    std::vector<uint8_t> section = MakeConstantSectionV00(declaredCount, vecMetadata, constant);
    ASSERT_EQ(CreateConstantDecoder(section.data(), section.size()), nullptr);
    EXPECT_TRUE(logger.contains({"VerifyConstant", "Constant metadata offset/size exceeds section bounds at index 0"}));
}

TEST(CppVerify, BadSparsityDimensionRejected) {
    Logger logger;
    const uint64_t declaredCount = 1;
    const std::vector<ConstantMetaDataV00> vecMetaData = {
        ConstantMetaDataV00{
            2,  // mrtIndex
            -5, // sparsityDimension (invalid)
            1,  // size
            0,  // offset
        },
    };
    const std::vector<uint8_t> constant = {'a'};
    std::vector<uint8_t> section = MakeConstantSectionV00(declaredCount, vecMetaData, constant);
    ASSERT_EQ(CreateConstantDecoder(section.data(), section.size()), nullptr);
    EXPECT_TRUE(logger.contains({"VerifyConstant", "Constant sparsity dimension is invalid at index 0"}));
}

TEST(CppVerify, EmptyConstantSection) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();
    std::unique_ptr<HeaderDecoder> headerDecoder =
        CreateHeaderDecoder(data.c_str(), static_cast<uint64_t>(HeaderSize()), static_cast<uint64_t>(data.size()));
    ASSERT_NE(headerDecoder, nullptr);

    std::unique_ptr<ConstantDecoder> decoder =
        CreateConstantDecoder(data.c_str() + headerDecoder->GetConstantsOffset(), headerDecoder->GetConstantsSize());
    ASSERT_NE(decoder, nullptr);

    ASSERT_TRUE(decoder->size() == 0);
}

TEST(CppVerify, LegacyConstantSectionSizeWrapRejected) {
    Logger logger;
    const uint64_t constantOffset = 80;
    const auto constantSize = static_cast<uint64_t>(SIZE_MAX_VALUE);
    const size_t vgfSize = 162;
    const auto constant = std::vector<uint8_t>(40, 'a');

    std::vector<uint8_t> buffer(vgfSize, 0);
    Header header({0, 0}, {0, 0}, {0, 0}, {constantOffset, constantSize}, pretendVulkanHeaderVersion);
    std::memcpy(buffer.data(), &header, sizeof(Header));
    if (constantOffset + constant.size() <= vgfSize) {
        std::memcpy(buffer.data() + constantOffset, constant.data(), constant.size());
    }

    EXPECT_EQ(nullptr, CreateHeaderDecoder(buffer.data(), static_cast<uint64_t>(HeaderSize()),
                                           static_cast<uint64_t>(buffer.size())));
    EXPECT_TRUE(logger.contains({"section bounds invalid"}));
    EXPECT_EQ(nullptr, CreateConstantDecoder(buffer.data() + constantOffset, constantSize));
    EXPECT_TRUE(logger.contains({"VerifyLegacyConstant", "size out of bounds"}));
}

TEST(CppVerify, LegacyConstantFlatbufferVerifyRejected) {
    Logger logger;
    std::array<uint8_t, 32> buffer{};
    buffer.fill(0xFF);

    EXPECT_EQ(nullptr, CreateConstantDecoder(buffer.data(), buffer.size()));
    EXPECT_TRUE(logger.contains({"VerifyLegacyConstant", "verification failed"}));
}

TEST(CppVerify, LegacyConstantMisalignedRejected) {
    Logger logger;
    const uint64_t constantOffset = 87;
    const uint64_t constantSize = UINT32_MAX_VALUE;
    const size_t vgfSize = 144;
    const auto constant = std::vector<uint8_t>(8, 'a');

    std::vector<uint8_t> buffer(vgfSize, 0);
    Header header({0, 0}, {0, 0}, {0, 0}, {constantOffset, constantSize}, pretendVulkanHeaderVersion);
    std::memcpy(buffer.data(), &header, sizeof(Header));
    if (constantOffset + constant.size() <= vgfSize) {
        std::memcpy(buffer.data() + constantOffset, constant.data(), constant.size());
    }

    EXPECT_EQ(nullptr, CreateHeaderDecoder(buffer.data(), static_cast<uint64_t>(HeaderSize()),
                                           static_cast<uint64_t>(buffer.size())));
    EXPECT_TRUE(logger.contains({"section bounds invalid"}));
    EXPECT_EQ(nullptr, CreateConstantDecoder(buffer.data() + constantOffset, constantSize));
    EXPECT_TRUE(logger.contains({"VerifyLegacyConstant", "data alignment invalid"}));
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
        mlsdk_decoder_create_header_decoder(data.c_str(), static_cast<uint64_t>(mlsdk_decoder_header_size()),
                                            static_cast<uint64_t>(data.size()), headerDecoderMemory.data());
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

    std::vector<uint8_t> constantDecoderMemory;
    constantDecoderMemory.resize(mlsdk_decoder_constant_table_decoder_mem_reqs());
    mlsdk_decoder_constant_table_decoder *decoder = mlsdk_decoder_create_constant_table_decoder(
        data.c_str() + modelConstantsSection.offset, modelConstantsSection.size, constantDecoderMemory.data());
    ASSERT_NE(decoder, nullptr);

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
        mlsdk_decoder_create_header_decoder(data.c_str(), static_cast<uint64_t>(mlsdk_decoder_header_size()),
                                            static_cast<uint64_t>(data.size()), headerDecoderMemory.data());
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

    std::vector<uint8_t> constantDecoderMemory;
    constantDecoderMemory.resize(mlsdk_decoder_constant_table_decoder_mem_reqs());
    mlsdk_decoder_constant_table_decoder *decoder = mlsdk_decoder_create_constant_table_decoder(
        data.c_str() + modelConstantsSection.offset, modelConstantsSection.size, constantDecoderMemory.data());
    ASSERT_NE(decoder, nullptr);

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
    const uint32_t numLargeConsts = 10;
    const size_t smallConstsSize = 2000; // 2KB
    const std::vector<uint8_t> smallConst(smallConstsSize, 's');
    const uint32_t numSmallConsts = 1000;
    const size_t veryLargeConstsSize = 500000000; // 500MB
    const std::vector<uint8_t> veryLargeConst(veryLargeConstsSize, 'L');
    const uint32_t numVeryLargeConsts = 4;
    const size_t verySmallConstsSize = 1; // 1B
    const std::vector<uint8_t> verySmallConst(verySmallConstsSize, 'S');
    const uint32_t numVerySmallConsts = 10000;

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
    EXPECT_GT(mmapped.size(), 2 * GB);

    std::vector<uint8_t> headerDecoderMemory;
    headerDecoderMemory.resize(mlsdk_decoder_header_decoder_mem_reqs());
    mlsdk_decoder_header_decoder *headerDecoder =
        mlsdk_decoder_create_header_decoder(mmapped.ptr(), static_cast<uint64_t>(mlsdk_decoder_header_size()),
                                            static_cast<uint64_t>(mmapped.size()), headerDecoderMemory.data());
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

    std::vector<uint8_t> constantDecoderMemory;
    constantDecoderMemory.resize(mlsdk_decoder_constant_table_decoder_mem_reqs());
    mlsdk_decoder_constant_table_decoder *decoder = mlsdk_decoder_create_constant_table_decoder(
        mmapped.ptr(modelConstantsSection.offset), modelConstantsSection.size, constantDecoderMemory.data());
    ASSERT_NE(decoder, nullptr);

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

TEST(CVerify, BadData) {

    uint8_t badData[16] = {0xde, 0xad, 0xbe, 0xef, 0xba, 0xad, 0xf0, 0x0d,
                           0xca, 0xfe, 0xba, 0xbe, 0x00, 0x11, 0x22, 0x33};

    std::vector<uint8_t> decoderMemory(mlsdk_decoder_constant_table_decoder_mem_reqs());

    ASSERT_EQ(mlsdk_decoder_create_constant_table_decoder(static_cast<const void *>(badData), sizeof(badData),
                                                          decoderMemory.data()),
              nullptr);
}

TEST(CVerify, ConstantHeaderTooSmallRejected) {
    Logger logger;
    std::array<uint8_t, 2> buffer{0, 0};

    std::vector<uint8_t> decoderMemory(mlsdk_decoder_constant_table_decoder_mem_reqs());
    EXPECT_EQ(nullptr, mlsdk_decoder_create_constant_table_decoder(buffer.data(), buffer.size(), decoderMemory.data()));
    EXPECT_TRUE(logger.contains({"Constant section too small to contain version"}));
}

TEST(CVerify, ConstantSizeCapRejected) {
#if SIZE_MAX < UINT64_MAX
    Logger logger;
    const uint64_t constantSize = SIZE_MAX_VALUE;
    std::array<uint8_t, CONSTANT_SECTION_VERSION_SIZE> section{};
    std::memcpy(section.data(), CONSTANT_SECTION_VERSION, CONSTANT_SECTION_VERSION_SIZE);

    std::vector<uint8_t> decoderMemory(mlsdk_decoder_constant_table_decoder_mem_reqs());
    EXPECT_EQ(nullptr, mlsdk_decoder_create_constant_table_decoder(section.data(), constantSize, decoderMemory.data()));
    EXPECT_TRUE(logger.contains({"VerifyConstant", "Size out of bounds"}));
#else
    GTEST_SKIP() << "Size cap guard only compiled when size_t is narrower than uint64_t";
#endif
}

TEST(CVerify, SectionTooSmallForMetadataRejected) {
    Logger logger;
    std::vector<uint8_t> section = MakeConstantSectionV00(1, {ConstantMetaDataV00{}}, {});
    section.resize(CONSTANT_SECTION_METADATA_OFFSET - 1);

    std::vector<uint8_t> decoderMemory(mlsdk_decoder_constant_table_decoder_mem_reqs());
    ASSERT_EQ(mlsdk_decoder_create_constant_table_decoder(section.data(), section.size(), decoderMemory.data()),
              nullptr);
    EXPECT_TRUE(logger.contains({"VerifyConstant", "Constant section too small to contain metadata"}));
}

TEST(CVerify, ConstantDataOffsetOverflowRejected) {
    Logger logger;
    std::array<uint8_t, CONSTANT_SECTION_METADATA_OFFSET> buffer{};
    std::memcpy(buffer.data(), CONSTANT_SECTION_VERSION, CONSTANT_SECTION_VERSION_SIZE);
    const uint64_t declaredCount = UINT64_MAX;
    std::memcpy(buffer.data() + CONSTANT_SECTION_COUNT_OFFSET, &declaredCount, sizeof(declaredCount));

    std::vector<uint8_t> decoderMemory(mlsdk_decoder_constant_table_decoder_mem_reqs());
    EXPECT_EQ(nullptr, mlsdk_decoder_create_constant_table_decoder(buffer.data(), static_cast<uint64_t>(buffer.size()),
                                                                   decoderMemory.data()));
    EXPECT_TRUE(logger.contains({"VerifyConstant", "Constant section declares more entries than fit in the buffer"}));
}

TEST(CVerify, DeclaredCountExceedingAvailableMetadataRejected) {
    Logger logger;
    const std::vector<ConstantMetaDataV00> vecMetaData = {
        ConstantMetaDataV00{
            7,  // mrtIndex
            -1, // sparsityDimension
            1,  // size
            0,  // offset
        },
    };
    const std::vector<uint8_t> constant = {'a'};
    const uint64_t declaredCount = 2;
    std::vector<uint8_t> section = MakeConstantSectionV00(declaredCount, vecMetaData, constant);

    std::vector<uint8_t> decoderMemory(mlsdk_decoder_constant_table_decoder_mem_reqs());
    ASSERT_EQ(mlsdk_decoder_create_constant_table_decoder(section.data(), section.size(), decoderMemory.data()),
              nullptr);
    EXPECT_TRUE(logger.contains({"VerifyConstant", "Constant section declares more entries than fit in the buffer"}));
}

TEST(CVerify, OutOfRangeOffsetsRejected) {
    Logger logger;
    const std::vector<ConstantMetaDataV00> vecMetadata = {
        ConstantMetaDataV00{
            3,  // mrtIndex
            -1, // sparsityDimension
            10, // size
            5,  // offset
        },
    };
    const std::vector<uint8_t> constant = {'a', 'b', 'c', 'd', 'e'};
    const uint64_t declaredCount = 1;
    std::vector<uint8_t> section = MakeConstantSectionV00(declaredCount, vecMetadata, constant);

    std::vector<uint8_t> decoderMemory(mlsdk_decoder_constant_table_decoder_mem_reqs());
    ASSERT_EQ(mlsdk_decoder_create_constant_table_decoder(section.data(), section.size(), decoderMemory.data()),
              nullptr);
    EXPECT_TRUE(logger.contains({"VerifyConstant", "Constant metadata offset/size exceeds section bounds at index 0"}));
}

TEST(CVerify, BadSparsityDimensionRejected) {
    Logger logger;
    const uint64_t declaredCount = 1;
    const std::vector<ConstantMetaDataV00> vecMetaData = {
        ConstantMetaDataV00{
            2,  // mrtIndex
            -5, // sparsityDimension (invalid)
            1,  // size
            0,  // offset
        },
    };
    const std::vector<uint8_t> constant = {'a'};
    std::vector<uint8_t> section = MakeConstantSectionV00(declaredCount, vecMetaData, constant);

    std::vector<uint8_t> decoderMemory(mlsdk_decoder_constant_table_decoder_mem_reqs());
    ASSERT_EQ(mlsdk_decoder_create_constant_table_decoder(section.data(), section.size(), decoderMemory.data()),
              nullptr);
    EXPECT_TRUE(logger.contains({"VerifyConstant", "Constant sparsity dimension is invalid at index 0"}));
}

TEST(CVerify, EmptyConstantSection) {
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

    std::vector<uint8_t> constantDecoderMemory;
    constantDecoderMemory.resize(mlsdk_decoder_constant_table_decoder_mem_reqs());
    mlsdk_decoder_constant_table_decoder *decoder = mlsdk_decoder_create_constant_table_decoder(
        data.c_str() + modelConstantsSection.offset, modelConstantsSection.size, constantDecoderMemory.data());
    ASSERT_NE(decoder, nullptr);

    ASSERT_TRUE(mlsdk_decoder_get_constant_table_num_entries(decoder) == 0);
}

TEST(CVerify, LegacyConstantSectionSizeWrapRejected) {
    Logger logger;
    const uint64_t constantOffset = 80;
    const auto constantSize = static_cast<uint64_t>(SIZE_MAX_VALUE);
    const size_t vgfSize = 162;
    const auto constant = std::vector<uint8_t>(40, 'a');

    std::vector<uint8_t> buffer(vgfSize, 0);
    Header header({0, 0}, {0, 0}, {0, 0}, {constantOffset, constantSize}, pretendVulkanHeaderVersion);
    std::memcpy(buffer.data(), &header, sizeof(Header));
    if (constantOffset + constant.size() <= vgfSize) {
        std::memcpy(buffer.data() + constantOffset, constant.data(), constant.size());
    }

    std::vector<uint8_t> headerDecoderMemory(mlsdk_decoder_header_decoder_mem_reqs());
    EXPECT_EQ(nullptr,
              mlsdk_decoder_create_header_decoder(buffer.data(), static_cast<uint64_t>(mlsdk_decoder_header_size()),
                                                  static_cast<uint64_t>(buffer.size()), headerDecoderMemory.data()));
    EXPECT_TRUE(logger.contains({"section bounds invalid"}));
    std::vector<uint8_t> decoderMemory(mlsdk_decoder_constant_table_decoder_mem_reqs());
    EXPECT_EQ(nullptr, mlsdk_decoder_create_constant_table_decoder(buffer.data() + constantOffset, constantSize,
                                                                   decoderMemory.data()));
    EXPECT_TRUE(logger.contains({"VerifyLegacyConstant", "size out of bounds"}));
}

TEST(CVerify, LegacyConstantFlatbufferVerifyRejected) {
    Logger logger;
    std::array<uint8_t, 32> buffer{};
    buffer.fill(0xFF);

    std::vector<uint8_t> decoderMemory(mlsdk_decoder_constant_table_decoder_mem_reqs());
    EXPECT_EQ(nullptr, mlsdk_decoder_create_constant_table_decoder(buffer.data(), buffer.size(), decoderMemory.data()));
    EXPECT_TRUE(logger.contains({"VerifyLegacyConstant", "verification failed"}));
}

TEST(CVerify, LegacyConstantMisalignedRejected) {
    Logger logger;
    const uint64_t constantOffset = 87;
    const uint64_t constantSize = UINT32_MAX_VALUE;
    const size_t vgfSize = 144;
    const auto constant = std::vector<uint8_t>(8, 'a');

    std::vector<uint8_t> buffer(vgfSize, 0);
    Header header({0, 0}, {0, 0}, {0, 0}, {constantOffset, constantSize}, pretendVulkanHeaderVersion);
    std::memcpy(buffer.data(), &header, sizeof(Header));
    if (constantOffset + constant.size() <= vgfSize) {
        std::memcpy(buffer.data() + constantOffset, constant.data(), constant.size());
    }

    std::vector<uint8_t> headerDecoderMemory(mlsdk_decoder_header_decoder_mem_reqs());
    EXPECT_EQ(nullptr,
              mlsdk_decoder_create_header_decoder(buffer.data(), static_cast<uint64_t>(mlsdk_decoder_header_size()),
                                                  static_cast<uint64_t>(buffer.size()), headerDecoderMemory.data()));
    EXPECT_TRUE(logger.contains({"section bounds invalid"}));
    std::vector<uint8_t> decoderMemory(mlsdk_decoder_constant_table_decoder_mem_reqs());
    EXPECT_EQ(nullptr, mlsdk_decoder_create_constant_table_decoder(buffer.data() + constantOffset, constantSize,
                                                                   decoderMemory.data()));
    EXPECT_TRUE(logger.contains({"VerifyLegacyConstant", "data alignment invalid"}));
}

// TODO: different datatypes.
