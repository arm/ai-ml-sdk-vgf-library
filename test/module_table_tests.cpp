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

#include <sstream>

using namespace mlsdk::vgflib;

const uint16_t pretendVulkanHeaderVersion = 123;

TEST(CppModuleTable, Empty) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer) == true);

    std::string data = buffer.str();
    ASSERT_TRUE(data.size() >= HeaderSize());

    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(data.c_str());
    ASSERT_TRUE(headerDecoder->IsValid() == true);
    ASSERT_TRUE(headerDecoder->CheckVersion() == true);

    std::unique_ptr<ModuleTableDecoder> decoder =
        CreateModuleTableDecoder(data.c_str() + headerDecoder->GetModuleTableOffset());

    ASSERT_TRUE(decoder->size() == 0);
}

TEST(CppModuleTable, Single) {
    std::stringstream buffer;
    std::vector<uint32_t> code{1, 2, 3, 4};

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddModule(ModuleType::GRAPH, "test", "main", code);
    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer) == true);

    std::string vgf_data = buffer.str();
    ASSERT_TRUE(vgf_data.size() >= HeaderSize());

    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(vgf_data.c_str());
    ASSERT_TRUE(headerDecoder->IsValid() == true);
    ASSERT_TRUE(headerDecoder->CheckVersion() == true);

    uint32_t moduleIndex = module.reference;
    //! [ModuleTableDecodingSample0 begin]
    std::unique_ptr<ModuleTableDecoder> moduleDecoder =
        CreateModuleTableDecoder(vgf_data.c_str() + headerDecoder->GetModuleTableOffset());

    size_t numModules = moduleDecoder->size();

    ASSERT_TRUE(moduleIndex < numModules);

    DataView<uint32_t> moduleCode;
    if (moduleDecoder->hasSPIRV(moduleIndex)) {
        moduleCode = moduleDecoder->getModuleCode(moduleIndex);

        //...
    }
    //! [ModuleTableDecodingSample0 end]

    ASSERT_TRUE(numModules == 1);
    ASSERT_TRUE(moduleDecoder->getModuleType(moduleIndex) == ModuleType::GRAPH);
    ASSERT_TRUE(moduleDecoder->getModuleName(moduleIndex) == "test");
    ASSERT_TRUE(moduleDecoder->hasSPIRV(moduleIndex) == true);
    ASSERT_TRUE(moduleDecoder->getModuleEntryPoint(moduleIndex) == "main");
    ASSERT_TRUE(moduleDecoder->getModuleCode(moduleIndex) == DataView<uint32_t>(code.data(), code.size()));
}

TEST(CppModuleTable, Single2) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddPlaceholderModule(ModuleType::COMPUTE, "test", "main");
    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer) == true);

    std::string data = buffer.str();
    ASSERT_TRUE(data.size() >= HeaderSize());

    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(data.c_str());
    ASSERT_TRUE(headerDecoder->IsValid() == true);
    ASSERT_TRUE(headerDecoder->CheckVersion() == true);

    std::unique_ptr<ModuleTableDecoder> decoder =
        CreateModuleTableDecoder(data.c_str() + headerDecoder->GetModuleTableOffset());

    ASSERT_TRUE(decoder->size() == 1);
    ASSERT_TRUE(decoder->getModuleType(module.reference) == ModuleType::COMPUTE);
    ASSERT_TRUE(decoder->getModuleName(module.reference) == "test");
    ASSERT_TRUE(decoder->hasSPIRV(module.reference) == false);
    ASSERT_TRUE(decoder->getModuleEntryPoint(module.reference) == "main");
    ASSERT_TRUE(decoder->getModuleCode(module.reference) == DataView<uint32_t>());
}

TEST(CModuleTable, Empty) {
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
    ASSERT_TRUE(moduleSection.size > 0);
    ASSERT_TRUE(moduleSection.offset == HEADER_HEADER_SIZE_VALUE);

    std::vector<uint8_t> decoderMemory;
    decoderMemory.resize(mlsdk_decoder_module_table_decoder_mem_reqs());
    mlsdk_decoder_module_table_decoder *decoder =
        mlsdk_decoder_create_module_table_decoder(data.c_str() + moduleSection.offset, decoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_module_table_num_entries(decoder) == 0);
}

TEST(CModuleTable, Single) {
    std::stringstream buffer;
    std::vector<uint32_t> code{1, 2, 3, 4};

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddModule(ModuleType::GRAPH, "test", "main", code);
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
    ASSERT_TRUE(moduleSection.size > 0);
    ASSERT_TRUE(moduleSection.offset == HEADER_HEADER_SIZE_VALUE);

    std::vector<uint8_t> decoderMemory;
    decoderMemory.resize(mlsdk_decoder_module_table_decoder_mem_reqs());
    mlsdk_decoder_module_table_decoder *decoder =
        mlsdk_decoder_create_module_table_decoder(data.c_str() + moduleSection.offset, decoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_module_table_num_entries(decoder) == 1);
    ASSERT_TRUE(mlsdk_decoder_get_module_type(decoder, module.reference) == mlsdk_decoder_module_type_graph);
    ASSERT_TRUE(std::string_view(mlsdk_decoder_get_module_name(decoder, module.reference)) == "test");
    ASSERT_TRUE(std::string_view(mlsdk_decoder_get_module_entry_point(decoder, module.reference)) == "main");

    mlsdk_decoder_spirv_code spirv;
    mlsdk_decoder_get_module_code(decoder, module.reference, &spirv);
    ASSERT_TRUE(DataView<uint32_t>(spirv.code, spirv.words) == DataView<uint32_t>(code.data(), code.size()));
}

TEST(CModuleTable, Single2) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddPlaceholderModule(ModuleType::COMPUTE, "test", "main");
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
    ASSERT_TRUE(moduleSection.size > 0);
    ASSERT_TRUE(moduleSection.offset == HEADER_HEADER_SIZE_VALUE);

    std::vector<uint8_t> decoderMemory;
    decoderMemory.resize(mlsdk_decoder_module_table_decoder_mem_reqs());
    mlsdk_decoder_module_table_decoder *decoder =
        mlsdk_decoder_create_module_table_decoder(data.c_str() + moduleSection.offset, decoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_module_table_num_entries(decoder) == 1);
    ASSERT_TRUE(mlsdk_decoder_get_module_type(decoder, module.reference) == mlsdk_decoder_module_type_compute);
    ASSERT_TRUE(std::string_view(mlsdk_decoder_get_module_name(decoder, module.reference)) == "test");
    ASSERT_TRUE(std::string_view(mlsdk_decoder_get_module_entry_point(decoder, module.reference)) == "main");

    mlsdk_decoder_spirv_code spirv;
    mlsdk_decoder_get_module_code(decoder, module.reference, &spirv);
    ASSERT_TRUE(spirv.code == nullptr);
    ASSERT_TRUE(spirv.words == 0);
}
