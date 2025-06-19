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

TEST(CppModelSequenceTable, SegmentInfo) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddPlaceholderModule(ModuleType::GRAPH, "test_module", "entry_point");

    SegmentInfoRef segment = encoder->AddSegmentInfo(module, "test_segment");

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer) == true);

    std::string data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(data.c_str());
    ASSERT_TRUE(headerDecoder->IsValid() == true);
    ASSERT_TRUE(headerDecoder->CheckVersion() == true);

    ASSERT_TRUE(VerifyModelSequenceTable(data.c_str() + headerDecoder->GetModelSequenceTableOffset(),
                                         headerDecoder->GetModelSequenceTableSize()));
    std::unique_ptr<ModelSequenceTableDecoder> decoder =
        CreateModelSequenceTableDecoder(data.c_str() + headerDecoder->GetModelSequenceTableOffset());

    ASSERT_TRUE(decoder->modelSequenceTableSize() == 1);
    ASSERT_TRUE(decoder->getSegmentType(segment.reference) == ModuleType::GRAPH);
    ASSERT_TRUE(decoder->getSegmentName(segment.reference) == std::string("test_segment"));
    ASSERT_TRUE(decoder->getSegmentModuleIndex(segment.reference) == module.reference);
}

TEST(CppModelSequenceTable, DescripterSetInfo) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddPlaceholderModule(ModuleType::GRAPH, "test_module", "entry_point");

    DescriptorSetInfoRef descriptor = encoder->AddDescriptorSetInfo();
    std::vector<DescriptorSetInfoRef> descriptors = {descriptor};

    SegmentInfoRef segment = encoder->AddSegmentInfo(module, "test_segment", descriptors);

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer) == true);

    std::string data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(data.c_str());
    ASSERT_TRUE(headerDecoder->IsValid() == true);
    ASSERT_TRUE(headerDecoder->CheckVersion() == true);

    ASSERT_TRUE(VerifyModelSequenceTable(data.c_str() + headerDecoder->GetModelSequenceTableOffset(),
                                         headerDecoder->GetModelSequenceTableSize()));
    std::unique_ptr<ModelSequenceTableDecoder> decoder =
        CreateModelSequenceTableDecoder(data.c_str() + headerDecoder->GetModelSequenceTableOffset());

    ASSERT_TRUE(decoder->modelSequenceTableSize() == 1);
    ASSERT_TRUE(decoder->getSegmentDescriptorSetInfosSize(segment.reference) == 1);
}

TEST(CppModelSequenceTable, DescriptorBindingSlot) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    std::vector<unsigned int> code = {0, 1, 2, 3};
    //! [ModelSequenceTableEncodingSample0 begin]
    ModuleRef module = encoder->AddModule(ModuleType::GRAPH, "test_module", "entry_point", code);
    //! [ModelSequenceTableEncodingSample0 end]

    BindingSlotRef binding = encoder->AddBindingSlot(1, ResourceRef{2});
    std::vector<BindingSlotRef> bindings = {binding};

    DescriptorSetInfoRef descriptor = encoder->AddDescriptorSetInfo(bindings);
    std::vector<DescriptorSetInfoRef> descriptors = {descriptor};

    SegmentInfoRef segment = encoder->AddSegmentInfo(module, "test_segment", descriptors);

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer) == true);

    std::string vgf_data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(vgf_data.c_str());
    ASSERT_TRUE(headerDecoder->IsValid() == true);
    ASSERT_TRUE(headerDecoder->CheckVersion() == true);

    //! [ModelSequenceTableDecodingSample0 begin]
    ASSERT_TRUE(VerifyModelSequenceTable(vgf_data.c_str() + headerDecoder->GetModelSequenceTableOffset(),
                                         headerDecoder->GetModelSequenceTableSize()));
    std::unique_ptr<ModelSequenceTableDecoder> seqTableDecoder =
        CreateModelSequenceTableDecoder(vgf_data.c_str() + headerDecoder->GetModelSequenceTableOffset());
    //! [ModelSequenceTableDecodingSample0 end]

    ASSERT_TRUE(seqTableDecoder->modelSequenceTableSize() == 1);
    ASSERT_TRUE(seqTableDecoder->getSegmentDescriptorSetInfosSize(segment.reference) == 1);

    uint32_t segmentIndex = segment.reference;
    uint32_t discriptorSetInfoIndex = descriptor.reference;
    //! [BindingSlotDecodingSample0 begin]
    ASSERT_TRUE(segmentIndex < seqTableDecoder->modelSequenceTableSize());
    ASSERT_TRUE(discriptorSetInfoIndex < seqTableDecoder->getSegmentDescriptorSetInfosSize(segmentIndex));

    BindingSlotArrayHandle bindingSlotsHandle =
        seqTableDecoder->getDescriptorBindingSlotsHandle(segmentIndex, discriptorSetInfoIndex);
    size_t numSlots = seqTableDecoder->getBindingsSize(bindingSlotsHandle);
    //! [BindingSlotDecodingSample0 end]

    ASSERT_TRUE(numSlots == 1);

    uint32_t slotIndex = binding.reference;
    //! [BindingSlotDecodingSample1 begin]
    ASSERT_TRUE(slotIndex < numSlots);

    uint32_t bindingId = seqTableDecoder->getBindingSlotBinding(bindingSlotsHandle, slotIndex);
    uint32_t mrtIndex = seqTableDecoder->getBindingSlotMrtIndex(bindingSlotsHandle, slotIndex);
    //! [BindingSlotDecodingSample1 end]

    ASSERT_TRUE(bindingId == 1);
    ASSERT_TRUE(mrtIndex == 2);
}

TEST(CppModelSequenceTable, SegmentBindingSlot) {

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    //! [BindingSlotEncodingSample2 begin]
    ModuleRef module = encoder->AddPlaceholderModule(ModuleType::GRAPH, "test_module", "entry_point");
    //! [BindingSlotEncodingSample2 end]

    auto inputRef = ResourceRef{2};
    auto outputRef = ResourceRef{5};

    //! [BindingSlotEncodingSample0 begin]
    BindingSlotRef inputBinding = encoder->AddBindingSlot(0, inputRef);
    BindingSlotRef outputBinding = encoder->AddBindingSlot(1, outputRef);
    //! [BindingSlotEncodingSample0 end]

    //! [BindingSlotEncodingSample1 begin]
    std::vector<BindingSlotRef> inputBindings = {inputBinding};
    std::vector<BindingSlotRef> outputBindings = {outputBinding};

    DescriptorSetInfoRef inputDescriptorRef = encoder->AddDescriptorSetInfo(inputBindings);
    DescriptorSetInfoRef outputDescriptorRef = encoder->AddDescriptorSetInfo(outputBindings);

    std::vector<DescriptorSetInfoRef> descriptorRefs = {inputDescriptorRef, outputDescriptorRef};
    //! [BindingSlotEncodingSample1 end]

    //! [BindingSlotEncodingSample3 begin]
    SegmentInfoRef segment =
        encoder->AddSegmentInfo(module, "test_segment", descriptorRefs, inputBindings, outputBindings);
    //! [BindingSlotEncodingSample3 end]

    //! [BindingSlotEncodingSample4 begin]
    encoder->AddModelSequenceInputsOutputs(inputBindings, {"input"}, outputBindings, {"output"});
    //! [BindingSlotEncodingSample4 end]

    //! [BindingSlotEncodingSample5 begin]
    std::stringstream buffer;
    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer) == true);
    //! [BindingSlotEncodingSample5 end]

    std::string data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(data.c_str());
    ASSERT_TRUE(headerDecoder->IsValid() == true);
    ASSERT_TRUE(headerDecoder->CheckVersion() == true);

    ASSERT_TRUE(VerifyModelSequenceTable(data.c_str() + headerDecoder->GetModelSequenceTableOffset(),
                                         headerDecoder->GetModelSequenceTableSize()));
    std::unique_ptr<ModelSequenceTableDecoder> seqTableDecoder =
        CreateModelSequenceTableDecoder(data.c_str() + headerDecoder->GetModelSequenceTableOffset());

    ASSERT_TRUE(seqTableDecoder->modelSequenceTableSize() == 1);

    BindingSlotArrayHandle bindingSlotsHandle = seqTableDecoder->getSegmentInputBindingSlotsHandle(segment.reference);

    ASSERT_TRUE(seqTableDecoder->getBindingsSize(bindingSlotsHandle) == 1);
    ASSERT_TRUE(seqTableDecoder->getBindingsSize(bindingSlotsHandle) == 1);
    ASSERT_TRUE(seqTableDecoder->getBindingSlotBinding(bindingSlotsHandle, 0) == 0);
    ASSERT_TRUE(seqTableDecoder->getBindingSlotMrtIndex(bindingSlotsHandle, 0) == 2);

    bindingSlotsHandle = seqTableDecoder->getSegmentOutputBindingSlotsHandle(segment.reference);

    ASSERT_TRUE(seqTableDecoder->getBindingsSize(bindingSlotsHandle) == 1);
    ASSERT_TRUE(seqTableDecoder->getBindingSlotBinding(bindingSlotsHandle, 0) == 1);
    ASSERT_TRUE(seqTableDecoder->getBindingSlotMrtIndex(bindingSlotsHandle, 0) == 5);
}

TEST(CppModelSequenceTable, BindingSlot) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddPlaceholderModule(ModuleType::GRAPH, "test_module", "entry_point");

    BindingSlotRef inputBinding = encoder->AddBindingSlot(1, ResourceRef{2});
    std::vector<BindingSlotRef> inputBindings = {inputBinding};

    BindingSlotRef outputBinding = encoder->AddBindingSlot(4, ResourceRef{5});
    std::vector<BindingSlotRef> outputBindings = {outputBinding};

    encoder->AddModelSequenceInputsOutputs(inputBindings, {"input_0"}, outputBindings, {});

    encoder->AddSegmentInfo(module, "test_segment", {}, inputBindings, outputBindings);

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer) == true);

    std::string data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(data.c_str());
    ASSERT_TRUE(headerDecoder->IsValid() == true);
    ASSERT_TRUE(headerDecoder->CheckVersion() == true);

    ASSERT_TRUE(VerifyModelSequenceTable(data.c_str() + headerDecoder->GetModelSequenceTableOffset(),
                                         headerDecoder->GetModelSequenceTableSize()));
    std::unique_ptr<ModelSequenceTableDecoder> seqTableDecoder =
        CreateModelSequenceTableDecoder(data.c_str() + headerDecoder->GetModelSequenceTableOffset());

    ASSERT_TRUE(seqTableDecoder->modelSequenceTableSize() == 1);

    BindingSlotArrayHandle inputsHandle = seqTableDecoder->getModelSequenceInputBindingSlotsHandle();
    ASSERT_TRUE(seqTableDecoder->getBindingsSize(inputsHandle) == 1);
    ASSERT_TRUE(seqTableDecoder->getBindingSlotBinding(inputsHandle, 0) == 1);
    ASSERT_TRUE(seqTableDecoder->getBindingSlotMrtIndex(inputsHandle, 0) == 2);

    NameArrayHandle inputNames = seqTableDecoder->getModelSequenceInputNamesHandle();
    ASSERT_TRUE(seqTableDecoder->getNamesSize(inputNames) == 1);
    ASSERT_TRUE(seqTableDecoder->getName(inputNames, 0) == "input_0");

    NameArrayHandle outputNames = seqTableDecoder->getModelSequenceOutputNamesHandle();
    ASSERT_TRUE(seqTableDecoder->getNamesSize(outputNames) == 0);

    BindingSlotArrayHandle outputsHandle = seqTableDecoder->getModelSequenceOutputBindingSlotsHandle();
    ASSERT_TRUE(seqTableDecoder->getBindingsSize(outputsHandle) == 1);
    ASSERT_TRUE(seqTableDecoder->getBindingSlotBinding(outputsHandle, 0) == 4);
    ASSERT_TRUE(seqTableDecoder->getBindingSlotMrtIndex(outputsHandle, 0) == 5);
}

TEST(CppModelSequenceTable, SegmentConstants) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddPlaceholderModule(ModuleType::GRAPH, "test_module", "entry_point");

    std::vector<ConstantRef> constants = {{1}, {2}, {3}};

    SegmentInfoRef segment = encoder->AddSegmentInfo(module, "test_segment", {}, {}, {}, constants);

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer) == true);

    std::string data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(data.c_str());
    ASSERT_TRUE(headerDecoder->IsValid() == true);
    ASSERT_TRUE(headerDecoder->CheckVersion() == true);

    ASSERT_TRUE(VerifyModelSequenceTable(data.c_str() + headerDecoder->GetModelSequenceTableOffset(),
                                         headerDecoder->GetModelSequenceTableSize()));
    std::unique_ptr<ModelSequenceTableDecoder> decoder =
        CreateModelSequenceTableDecoder(data.c_str() + headerDecoder->GetModelSequenceTableOffset());

    ASSERT_TRUE(decoder->modelSequenceTableSize() == 1);

    DataView<uint32_t> constantIndexes = decoder->getSegmentConstantIndexes(segment.reference);
    ASSERT_TRUE(constantIndexes.size() == 3);
    ASSERT_TRUE(constantIndexes[0] == constants[0].reference);
    ASSERT_TRUE(constantIndexes[1] == constants[1].reference);
    ASSERT_TRUE(constantIndexes[2] == constants[2].reference);
}

TEST(CppModelSequenceTable, SegmentDispatchShape) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddPlaceholderModule(ModuleType::GRAPH, "test_module", "entry_point");

    std::array<uint32_t, 3> dispatchShape = {1, 2, 3};

    SegmentInfoRef segment = encoder->AddSegmentInfo(module, "test_segment", {}, {}, {}, {}, dispatchShape);

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer) == true);

    std::string data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(data.c_str());
    ASSERT_TRUE(headerDecoder->IsValid() == true);
    ASSERT_TRUE(headerDecoder->CheckVersion() == true);

    ASSERT_TRUE(VerifyModelSequenceTable(data.c_str() + headerDecoder->GetModelSequenceTableOffset(),
                                         headerDecoder->GetModelSequenceTableSize()));
    std::unique_ptr<ModelSequenceTableDecoder> decoder =
        CreateModelSequenceTableDecoder(data.c_str() + headerDecoder->GetModelSequenceTableOffset());

    ASSERT_TRUE(decoder->modelSequenceTableSize() == 1);

    DataView<uint32_t> checkShape = decoder->getSegmentDispatchShape(segment.reference);
    ASSERT_TRUE(checkShape.size() == 3);
    ASSERT_TRUE(checkShape[0] == dispatchShape[0]);
    ASSERT_TRUE(checkShape[1] == dispatchShape[1]);
    ASSERT_TRUE(checkShape[2] == dispatchShape[2]);
}

TEST(CppModelSequenceTable, PushConstantRange) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddPlaceholderModule(ModuleType::GRAPH, "test_module", "entry_point");

    PushConstRangeRef pushConstRange = encoder->AddPushConstRange(1, 2, 3);
    std::vector<PushConstRangeRef> pushConstRanges = {pushConstRange};

    SegmentInfoRef segment = encoder->AddSegmentInfo(module, "test_segment", {}, {}, {}, {}, {}, pushConstRanges);

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer) == true);

    std::string data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(data.c_str());
    ASSERT_TRUE(headerDecoder->IsValid() == true);
    ASSERT_TRUE(headerDecoder->CheckVersion() == true);

    std::unique_ptr<ModelSequenceTableDecoder> seqTableDecoder =
        CreateModelSequenceTableDecoder(data.c_str() + headerDecoder->GetModelSequenceTableOffset());

    ASSERT_TRUE(VerifyModelSequenceTable(data.c_str() + headerDecoder->GetModelSequenceTableOffset(),
                                         headerDecoder->GetModelSequenceTableSize()));
    ASSERT_TRUE(seqTableDecoder->modelSequenceTableSize() == 1);

    PushConstantRangeHandle handle = seqTableDecoder->getSegmentPushConstRange(segment.reference);

    ASSERT_TRUE(seqTableDecoder->getPushConstRangesSize(handle) == 1);
    ASSERT_TRUE(seqTableDecoder->getPushConstRangeStageFlags(handle, pushConstRange.reference) == 1);
    ASSERT_TRUE(seqTableDecoder->getPushConstRangeOffset(handle, pushConstRange.reference) == 2);
    ASSERT_TRUE(seqTableDecoder->getPushConstRangeSize(handle, pushConstRange.reference) == 3);
}

TEST(CModelSequenceTable, SegmentInfo) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddPlaceholderModule(ModuleType::GRAPH, "test_module", "entry_point");

    SegmentInfoRef segment = encoder->AddSegmentInfo(module, "test_segment");

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
    ASSERT_TRUE(mlsdk_decoder_is_valid_module_table(data.c_str() + moduleSection.offset, moduleSection.size));

    std::vector<uint8_t> moduleTableDecoderMemory;
    moduleTableDecoderMemory.resize(mlsdk_decoder_module_table_decoder_mem_reqs());
    mlsdk_decoder_module_table_decoder *moduleTabledecoder =
        mlsdk_decoder_create_module_table_decoder(data.c_str() + moduleSection.offset, moduleTableDecoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_module_table_num_entries(moduleTabledecoder) == 1);

    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    ASSERT_TRUE(modelSequenceSection.size > 0);
    ASSERT_TRUE(modelSequenceSection.offset == HEADER_HEADER_SIZE_VALUE + moduleSection.size);
    ASSERT_TRUE(
        mlsdk_decoder_is_valid_model_sequence(data.c_str() + modelSequenceSection.offset, modelSequenceSection.size));

    std::vector<uint8_t> modelSequenceDecoderMemory;
    modelSequenceDecoderMemory.resize(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    mlsdk_decoder_model_sequence_decoder *modelSequenceDecoder = mlsdk_decoder_create_model_sequence_decoder(
        data.c_str() + modelSequenceSection.offset, modelSequenceDecoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_model_sequence_table_size(modelSequenceDecoder) == 1);
    ASSERT_TRUE(mlsdk_decoder_model_sequence_get_segment_type(modelSequenceDecoder, segment.reference) ==
                mlsdk_decoder_module_type_graph);
    ASSERT_TRUE(strcmp(mlsdk_decoder_model_sequence_get_segment_name(modelSequenceDecoder, segment.reference),
                       "test_segment") == 0);
    ASSERT_TRUE(mlsdk_decoder_model_sequence_get_segment_module_index(modelSequenceDecoder, segment.reference) ==
                module.reference);
}

TEST(CModelSequenceTable, DescripterSetInfo) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddPlaceholderModule(ModuleType::GRAPH, "test_module", "entry_point");

    DescriptorSetInfoRef descriptor = encoder->AddDescriptorSetInfo();
    std::vector<DescriptorSetInfoRef> descriptors = {descriptor};

    SegmentInfoRef segment = encoder->AddSegmentInfo(module, "test_segment", descriptors);

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
    ASSERT_TRUE(mlsdk_decoder_is_valid_module_table(data.c_str() + moduleSection.offset, moduleSection.size));

    std::vector<uint8_t> moduleTableDecoderMemory;
    moduleTableDecoderMemory.resize(mlsdk_decoder_module_table_decoder_mem_reqs());
    mlsdk_decoder_module_table_decoder *moduleTabledecoder =
        mlsdk_decoder_create_module_table_decoder(data.c_str() + moduleSection.offset, moduleTableDecoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_module_table_num_entries(moduleTabledecoder) == 1);

    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    ASSERT_TRUE(modelSequenceSection.size > 0);
    ASSERT_TRUE(modelSequenceSection.offset == HEADER_HEADER_SIZE_VALUE + moduleSection.size);
    ASSERT_TRUE(
        mlsdk_decoder_is_valid_model_sequence(data.c_str() + modelSequenceSection.offset, modelSequenceSection.size));

    std::vector<uint8_t> modelSequenceDecoderMemory;
    modelSequenceDecoderMemory.resize(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    mlsdk_decoder_model_sequence_decoder *modelSequenceDecoder = mlsdk_decoder_create_model_sequence_decoder(
        data.c_str() + modelSequenceSection.offset, modelSequenceDecoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_model_sequence_table_size(modelSequenceDecoder) == 1);
    ASSERT_TRUE(
        mlsdk_decoder_model_sequence_get_segment_descriptorset_info_size(modelSequenceDecoder, segment.reference) == 1);
}

TEST(CModelSequenceTable, DescriptorBindingSlot) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddPlaceholderModule(ModuleType::GRAPH, "test_module", "entry_point");

    BindingSlotRef binding = encoder->AddBindingSlot(1, ResourceRef{2});
    std::vector<BindingSlotRef> bindings = {binding};

    DescriptorSetInfoRef descriptor = encoder->AddDescriptorSetInfo(bindings);
    std::vector<DescriptorSetInfoRef> descriptors = {descriptor};

    SegmentInfoRef segment = encoder->AddSegmentInfo(module, "test_segment", descriptors);

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
    ASSERT_TRUE(mlsdk_decoder_is_valid_module_table(data.c_str() + moduleSection.offset, moduleSection.size));

    std::vector<uint8_t> moduleTableDecoderMemory;
    moduleTableDecoderMemory.resize(mlsdk_decoder_module_table_decoder_mem_reqs());
    mlsdk_decoder_module_table_decoder *moduleTabledecoder =
        mlsdk_decoder_create_module_table_decoder(data.c_str() + moduleSection.offset, moduleTableDecoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_module_table_num_entries(moduleTabledecoder) == 1);

    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    ASSERT_TRUE(modelSequenceSection.size > 0);
    ASSERT_TRUE(modelSequenceSection.offset == HEADER_HEADER_SIZE_VALUE + moduleSection.size);
    ASSERT_TRUE(
        mlsdk_decoder_is_valid_model_sequence(data.c_str() + modelSequenceSection.offset, modelSequenceSection.size));

    std::vector<uint8_t> modelSequenceDecoderMemory;
    modelSequenceDecoderMemory.resize(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    mlsdk_decoder_model_sequence_decoder *modelSequenceDecoder = mlsdk_decoder_create_model_sequence_decoder(
        data.c_str() + modelSequenceSection.offset, modelSequenceDecoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_model_sequence_table_size(modelSequenceDecoder) == 1);
    ASSERT_TRUE(
        mlsdk_decoder_model_sequence_get_segment_descriptorset_info_size(modelSequenceDecoder, segment.reference) == 1);

    mlsdk_decoder_binding_slots_handle handle = mlsdk_decoder_model_sequence_get_segment_descriptor_binding_slot(
        modelSequenceDecoder, segment.reference, descriptor.reference);

    ASSERT_TRUE(mlsdk_decoder_binding_slot_size(modelSequenceDecoder, handle) == 1);
    ASSERT_TRUE(mlsdk_decoder_binding_slot_binding_id(modelSequenceDecoder, handle, binding.reference) == 1);
    ASSERT_TRUE(mlsdk_decoder_binding_slot_mrt_index(modelSequenceDecoder, handle, binding.reference) == 2);
}

TEST(CModelSequenceTable, SegmentBindingSlot) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddPlaceholderModule(ModuleType::GRAPH, "test_module", "entry_point");

    BindingSlotRef inputBinding = encoder->AddBindingSlot(1, ResourceRef{2});
    std::vector<BindingSlotRef> inputBindings = {inputBinding};

    BindingSlotRef outputBinding = encoder->AddBindingSlot(4, ResourceRef{5});
    std::vector<BindingSlotRef> outputBindings = {outputBinding};

    SegmentInfoRef segment = encoder->AddSegmentInfo(module, "test_segment", {}, inputBindings, outputBindings);

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
    ASSERT_TRUE(mlsdk_decoder_is_valid_module_table(data.c_str() + moduleSection.offset, moduleSection.size));

    std::vector<uint8_t> moduleTableDecoderMemory;
    moduleTableDecoderMemory.resize(mlsdk_decoder_module_table_decoder_mem_reqs());
    mlsdk_decoder_module_table_decoder *moduleTabledecoder =
        mlsdk_decoder_create_module_table_decoder(data.c_str() + moduleSection.offset, moduleTableDecoderMemory.data());

    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    ASSERT_TRUE(modelSequenceSection.size > 0);
    ASSERT_TRUE(modelSequenceSection.offset == HEADER_HEADER_SIZE_VALUE + moduleSection.size);
    ASSERT_TRUE(
        mlsdk_decoder_is_valid_model_sequence(data.c_str() + modelSequenceSection.offset, modelSequenceSection.size));

    ASSERT_TRUE(mlsdk_decoder_get_module_table_num_entries(moduleTabledecoder) == 1);

    std::vector<uint8_t> modelSequenceDecoderMemory;
    modelSequenceDecoderMemory.resize(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    mlsdk_decoder_model_sequence_decoder *modelSequenceDecoder = mlsdk_decoder_create_model_sequence_decoder(
        data.c_str() + modelSequenceSection.offset, modelSequenceDecoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_model_sequence_table_size(modelSequenceDecoder) == 1);

    mlsdk_decoder_binding_slots_handle handle =
        mlsdk_decoder_model_sequence_get_segment_input_binding_slot(modelSequenceDecoder, segment.reference);

    ASSERT_TRUE(mlsdk_decoder_binding_slot_size(modelSequenceDecoder, handle) == 1);
    ASSERT_TRUE(mlsdk_decoder_binding_slot_binding_id(modelSequenceDecoder, handle, 0) == 1);
    ASSERT_TRUE(mlsdk_decoder_binding_slot_mrt_index(modelSequenceDecoder, handle, 0) == 2);

    handle = mlsdk_decoder_model_sequence_get_segment_output_binding_slot(modelSequenceDecoder, segment.reference);

    ASSERT_TRUE(mlsdk_decoder_binding_slot_size(modelSequenceDecoder, handle) == 1);
    ASSERT_TRUE(mlsdk_decoder_binding_slot_binding_id(modelSequenceDecoder, handle, 0) == 4);
    ASSERT_TRUE(mlsdk_decoder_binding_slot_mrt_index(modelSequenceDecoder, handle, 0) == 5);
}

TEST(CModelSequenceTable, BindingSlot) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddPlaceholderModule(ModuleType::GRAPH, "test_module", "entry_point");

    BindingSlotRef inputBinding = encoder->AddBindingSlot(1, ResourceRef{2});
    std::vector<BindingSlotRef> inputBindings = {inputBinding};

    BindingSlotRef outputBinding = encoder->AddBindingSlot(4, ResourceRef{5});
    std::vector<BindingSlotRef> outputBindings = {outputBinding};

    encoder->AddModelSequenceInputsOutputs(inputBindings, {}, outputBindings, {});

    encoder->AddSegmentInfo(module, "test_segment", {}, inputBindings, outputBindings);

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
    ASSERT_TRUE(mlsdk_decoder_is_valid_module_table(data.c_str() + moduleSection.offset, moduleSection.size));

    std::vector<uint8_t> moduleTableDecoderMemory;
    moduleTableDecoderMemory.resize(mlsdk_decoder_module_table_decoder_mem_reqs());
    mlsdk_decoder_module_table_decoder *moduleTabledecoder =
        mlsdk_decoder_create_module_table_decoder(data.c_str() + moduleSection.offset, moduleTableDecoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_module_table_num_entries(moduleTabledecoder) == 1);

    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    ASSERT_TRUE(modelSequenceSection.size > 0);
    ASSERT_TRUE(modelSequenceSection.offset == HEADER_HEADER_SIZE_VALUE + moduleSection.size);
    ASSERT_TRUE(
        mlsdk_decoder_is_valid_model_sequence(data.c_str() + modelSequenceSection.offset, modelSequenceSection.size));

    std::vector<uint8_t> modelSequenceDecoderMemory;
    modelSequenceDecoderMemory.resize(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    mlsdk_decoder_model_sequence_decoder *modelSequenceDecoder = mlsdk_decoder_create_model_sequence_decoder(
        data.c_str() + modelSequenceSection.offset, modelSequenceDecoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_model_sequence_table_size(modelSequenceDecoder) == 1);

    mlsdk_decoder_binding_slots_handle handle =
        mlsdk_decoder_model_sequence_get_input_binding_slot(modelSequenceDecoder);

    ASSERT_TRUE(mlsdk_decoder_binding_slot_size(modelSequenceDecoder, handle) == 1);
    ASSERT_TRUE(mlsdk_decoder_binding_slot_binding_id(modelSequenceDecoder, handle, 0) == 1);
    ASSERT_TRUE(mlsdk_decoder_binding_slot_mrt_index(modelSequenceDecoder, handle, 0) == 2);

    handle = mlsdk_decoder_model_sequence_get_output_binding_slot(modelSequenceDecoder);

    ASSERT_TRUE(mlsdk_decoder_binding_slot_size(modelSequenceDecoder, handle) == 1);
    ASSERT_TRUE(mlsdk_decoder_binding_slot_binding_id(modelSequenceDecoder, handle, 0) == 4);
    ASSERT_TRUE(mlsdk_decoder_binding_slot_mrt_index(modelSequenceDecoder, handle, 0) == 5);
}

TEST(CModelSequenceTable, SegmentConstants) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddPlaceholderModule(ModuleType::GRAPH, "test_module", "entry_point");

    std::vector<ConstantRef> constants = {{1}, {2}, {3}};

    SegmentInfoRef segment = encoder->AddSegmentInfo(module, "test_segment", {}, {}, {}, constants);

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
    ASSERT_TRUE(mlsdk_decoder_is_valid_module_table(data.c_str() + moduleSection.offset, moduleSection.size));

    std::vector<uint8_t> moduleTableDecoderMemory;
    moduleTableDecoderMemory.resize(mlsdk_decoder_module_table_decoder_mem_reqs());
    mlsdk_decoder_module_table_decoder *moduleTabledecoder =
        mlsdk_decoder_create_module_table_decoder(data.c_str() + moduleSection.offset, moduleTableDecoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_module_table_num_entries(moduleTabledecoder) == 1);

    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    ASSERT_TRUE(modelSequenceSection.size > 0);
    ASSERT_TRUE(modelSequenceSection.offset == HEADER_HEADER_SIZE_VALUE + moduleSection.size);
    ASSERT_TRUE(
        mlsdk_decoder_is_valid_model_sequence(data.c_str() + modelSequenceSection.offset, modelSequenceSection.size));

    std::vector<uint8_t> modelSequenceDecoderMemory;
    modelSequenceDecoderMemory.resize(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    mlsdk_decoder_model_sequence_decoder *modelSequenceDecoder = mlsdk_decoder_create_model_sequence_decoder(
        data.c_str() + modelSequenceSection.offset, modelSequenceDecoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_model_sequence_table_size(modelSequenceDecoder) == 1);

    mlsdk_decoder_constant_indexes constantIndexes = {};
    mlsdk_decoder_model_sequence_get_segment_constant_indexes(modelSequenceDecoder, segment.reference,
                                                              &constantIndexes);
    ASSERT_TRUE(constantIndexes.size == 3);
    ASSERT_TRUE(constantIndexes.data[0] == constants[0].reference);
    ASSERT_TRUE(constantIndexes.data[1] == constants[1].reference);
    ASSERT_TRUE(constantIndexes.data[2] == constants[2].reference);
}

TEST(CModelSequenceTable, SegmentDispatchShape) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddPlaceholderModule(ModuleType::GRAPH, "test_module", "entry_point");

    std::array<uint32_t, 3> dispatchShape = {1, 2, 3};

    SegmentInfoRef segment = encoder->AddSegmentInfo(module, "test_segment", {}, {}, {}, {}, dispatchShape);

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
    ASSERT_TRUE(mlsdk_decoder_is_valid_module_table(data.c_str() + moduleSection.offset, moduleSection.size));

    std::vector<uint8_t> moduleTableDecoderMemory;
    moduleTableDecoderMemory.resize(mlsdk_decoder_module_table_decoder_mem_reqs());
    mlsdk_decoder_module_table_decoder *moduleTabledecoder =
        mlsdk_decoder_create_module_table_decoder(data.c_str() + moduleSection.offset, moduleTableDecoderMemory.data());

    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    ASSERT_TRUE(modelSequenceSection.size > 0);
    ASSERT_TRUE(modelSequenceSection.offset == HEADER_HEADER_SIZE_VALUE + moduleSection.size);
    ASSERT_TRUE(
        mlsdk_decoder_is_valid_model_sequence(data.c_str() + modelSequenceSection.offset, modelSequenceSection.size));

    ASSERT_TRUE(mlsdk_decoder_get_module_table_num_entries(moduleTabledecoder) == 1);

    std::vector<uint8_t> modelSequenceDecoderMemory;
    modelSequenceDecoderMemory.resize(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    mlsdk_decoder_model_sequence_decoder *modelSequenceDecoder = mlsdk_decoder_create_model_sequence_decoder(
        data.c_str() + modelSequenceSection.offset, modelSequenceDecoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_model_sequence_table_size(modelSequenceDecoder) == 1);

    mlsdk_decoder_dispatch_shape checkShape;
    mlsdk_decoder_model_sequence_get_segment_dispatch_shape(modelSequenceDecoder, segment.reference, &checkShape);
    ASSERT_TRUE(checkShape.data[0] == dispatchShape[0]);
    ASSERT_TRUE(checkShape.data[1] == dispatchShape[1]);
    ASSERT_TRUE(checkShape.data[2] == dispatchShape[2]);
}

TEST(CModelSequenceTable, PushConstantRange) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddPlaceholderModule(ModuleType::GRAPH, "test_module", "entry_point");

    //! [PushConstRangesEncodingSample0 begin]
    PushConstRangeRef pushConstRange = encoder->AddPushConstRange(1, 2, 3);
    std::vector<PushConstRangeRef> pushConstRanges = {pushConstRange};
    //! [PushConstRangesEncodingSample0 end]

    SegmentInfoRef segment = encoder->AddSegmentInfo(module, "test_segment", {}, {}, {}, {}, {}, pushConstRanges);

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
    ASSERT_TRUE(mlsdk_decoder_is_valid_module_table(data.c_str() + moduleSection.offset, moduleSection.size));

    std::vector<uint8_t> moduleTableDecoderMemory;
    moduleTableDecoderMemory.resize(mlsdk_decoder_module_table_decoder_mem_reqs());
    mlsdk_decoder_module_table_decoder *moduleTabledecoder =
        mlsdk_decoder_create_module_table_decoder(data.c_str() + moduleSection.offset, moduleTableDecoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_module_table_num_entries(moduleTabledecoder) == 1);

    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    ASSERT_TRUE(modelSequenceSection.size > 0);
    ASSERT_TRUE(modelSequenceSection.offset == HEADER_HEADER_SIZE_VALUE + moduleSection.size);
    ASSERT_TRUE(
        mlsdk_decoder_is_valid_model_sequence(data.c_str() + modelSequenceSection.offset, modelSequenceSection.size));

    std::vector<uint8_t> modelSequenceDecoderMemory;
    modelSequenceDecoderMemory.resize(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    mlsdk_decoder_model_sequence_decoder *modelSequenceDecoder = mlsdk_decoder_create_model_sequence_decoder(
        data.c_str() + modelSequenceSection.offset, modelSequenceDecoderMemory.data());

    ASSERT_TRUE(mlsdk_decoder_get_model_sequence_table_size(modelSequenceDecoder) == 1);

    uint32_t segmentIndex = segment.reference;
    uint32_t rangeIdx = pushConstRange.reference;

    //! [PushConstRangesDecodingSample0 begin]
    mlsdk_decoder_push_constant_ranges_handle pcrHandle =
        mlsdk_decoder_model_sequence_get_segment_push_constant_range(modelSequenceDecoder, segmentIndex);

    size_t numRanges = mlsdk_decoder_get_push_constant_ranges_size(modelSequenceDecoder, pcrHandle);

    ASSERT_TRUE(numRanges > rangeIdx);

    uint32_t rangeOffset = mlsdk_decoder_get_push_constant_range_offset(modelSequenceDecoder, pcrHandle, rangeIdx);
    uint32_t rangeSize = mlsdk_decoder_get_push_constant_range_size(modelSequenceDecoder, pcrHandle, rangeIdx);
    uint32_t rangeStageFlags =
        mlsdk_decoder_get_push_constant_range_stage_flags(modelSequenceDecoder, pcrHandle, rangeIdx);

    //! [PushConstRangesDecodingSample0 end]

    ASSERT_TRUE(numRanges == 1);
    ASSERT_TRUE(rangeStageFlags == 1);
    ASSERT_TRUE(rangeOffset == 2);
    ASSERT_TRUE(rangeSize == 3);
}
