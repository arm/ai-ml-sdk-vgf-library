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

using namespace mlsdk::vgflib;

const uint16_t pretendVulkanHeaderVersion = 123;

TEST(CppEncodeDecode, HeaderTest) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string vgf_data = buffer.str();
    ASSERT_TRUE(vgf_data.size() >= HeaderSize());

    //! [HeaderDecodingSample0 begin]
    std::unique_ptr<HeaderDecoder> decoder =
        CreateHeaderDecoder(vgf_data.c_str(), static_cast<uint64_t>(vgf_data.size()));
    //! [HeaderDecodingSample0 end]

    //! [HeaderDecodingSample1 begin]
    ASSERT_TRUE(decoder->IsValid());
    ASSERT_TRUE(decoder->CheckVersion());
    //! [HeaderDecodingSample1 end]

    ASSERT_TRUE(decoder->GetEncoderVulkanHeadersVersion() == pretendVulkanHeaderVersion);

    ASSERT_TRUE(decoder->GetMajor() == HEADER_MAJOR_VERSION_VALUE);
    ASSERT_TRUE(decoder->GetMinor() == HEADER_MINOR_VERSION_VALUE);
    ASSERT_TRUE(decoder->GetPatch() == HEADER_PATCH_VERSION_VALUE);
    ASSERT_TRUE(decoder->IsLatestVersion());

    ASSERT_TRUE(decoder->GetModuleTableSize() > 0);
    ASSERT_TRUE(decoder->GetModuleTableOffset() == HEADER_HEADER_SIZE_VALUE);

    ASSERT_TRUE(decoder->GetModelSequenceTableSize() > 0);
    ASSERT_TRUE(decoder->GetModelSequenceTableOffset() == HEADER_HEADER_SIZE_VALUE + decoder->GetModuleTableSize());

    ASSERT_TRUE(decoder->GetModelResourceTableSize() > 0);
    ASSERT_TRUE(decoder->GetModelResourceTableOffset() ==
                decoder->GetModelSequenceTableOffset() + decoder->GetModelSequenceTableSize());

    ASSERT_TRUE(decoder->GetConstantsSize() > 0);
    ASSERT_TRUE(decoder->GetConstantsOffset() ==
                decoder->GetModelResourceTableOffset() + decoder->GetModelResourceTableSize());
}

TEST(CppDecode, WrongMagic) {
    std::array<char, HEADER_HEADER_SIZE_VALUE> data = {0};
    std::unique_ptr<HeaderDecoder> decoder = CreateHeaderDecoder(data.data(), static_cast<uint64_t>(data.size()));
    ASSERT_TRUE(decoder->IsValid() == false);
    ASSERT_TRUE(decoder->CheckVersion() == false);

    // Test the FourCC generator used for the magic value
    FourCCValue vgf1 = FourCC('V', 'G', 'F', '1');
    ASSERT_TRUE(vgf1.a == 'V');
    ASSERT_TRUE(vgf1.b == 'G');
    ASSERT_TRUE(vgf1.c == 'F');
    ASSERT_TRUE(vgf1.d == '1');

    // Test equality operator
    ASSERT_TRUE(vgf1 == FourCC('V', 'G', 'F', '1'));
}

TEST(CppEncode, FailToWrite) {
    std::stringbuf roBuf("", std::ios_base::in); // read-only stream
    std::ostream roStream(&roBuf);

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(roStream) == false);
}

TEST(CDecode, HeaderTest) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();
    ASSERT_TRUE(data.size() >= mlsdk_decoder_header_size());

    std::vector<uint8_t> decoderMemory;
    decoderMemory.resize(mlsdk_decoder_header_decoder_mem_reqs());
    mlsdk_decoder_header_decoder *decoder =
        mlsdk_decoder_create_header_decoder(data.c_str(), static_cast<uint64_t>(data.size()), decoderMemory.data());
    ASSERT_TRUE(mlsdk_decoder_is_header_valid(decoder));
    ASSERT_TRUE(mlsdk_decoder_is_header_compatible(decoder));

    mlsdk_vk_header_version vkHeaderVersion;
    mlsdk_decoder_get_encoder_vk_header_version(decoder, &vkHeaderVersion);
    ASSERT_TRUE(vkHeaderVersion == pretendVulkanHeaderVersion);

    mlsdk_decoder_vgf_version version;
    mlsdk_decoder_get_header_version(decoder, &version);
    ASSERT_TRUE(version.major == HEADER_MAJOR_VERSION_VALUE);
    ASSERT_TRUE(version.minor == HEADER_MINOR_VERSION_VALUE);
    ASSERT_TRUE(version.patch == HEADER_PATCH_VERSION_VALUE);

    ASSERT_TRUE(mlsdk_decoder_is_latest_version(decoder));

    mlsdk_decoder_vgf_section_info moduleSection;
    mlsdk_decoder_get_header_section_info(decoder, mlsdk_decoder_section_modules, &moduleSection);
    ASSERT_TRUE(moduleSection.size > 0);
    ASSERT_TRUE(moduleSection.offset == HEADER_HEADER_SIZE_VALUE);

    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(decoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    ASSERT_TRUE(modelSequenceSection.size > 0);
    ASSERT_TRUE(modelSequenceSection.offset == HEADER_HEADER_SIZE_VALUE + moduleSection.size);

    mlsdk_decoder_vgf_section_info modelResourceSection;
    mlsdk_decoder_get_header_section_info(decoder, mlsdk_decoder_section_resources, &modelResourceSection);
    ASSERT_TRUE(modelResourceSection.size > 0);
    ASSERT_TRUE(modelResourceSection.offset == modelSequenceSection.offset + modelSequenceSection.size);

    mlsdk_decoder_vgf_section_info modelConstantsSection;
    mlsdk_decoder_get_header_section_info(decoder, mlsdk_decoder_section_constants, &modelConstantsSection);
    ASSERT_TRUE(modelConstantsSection.size > 0);
    ASSERT_TRUE(modelConstantsSection.offset == modelResourceSection.offset + modelResourceSection.size);
}

TEST(CDecode, WrongMagic) {
    std::array<char, HEADER_HEADER_SIZE_VALUE> data = {0};

    std::vector<uint8_t> decoderMemory;
    decoderMemory.resize(mlsdk_decoder_header_decoder_mem_reqs());
    mlsdk_decoder_header_decoder *decoder =
        mlsdk_decoder_create_header_decoder(data.data(), static_cast<uint64_t>(data.size()), decoderMemory.data());
    ASSERT_TRUE(mlsdk_decoder_is_header_valid(decoder) == false);
    ASSERT_TRUE(mlsdk_decoder_is_header_compatible(decoder) == false);
}
