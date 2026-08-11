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
#include "vgf_generated.h"

#include <gtest/gtest.h>

#include <flatbuffers/flatbuffer_builder.h>

#include <array>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace mlsdk::vgflib;
using logging::utils::Logger;

namespace {

const uint16_t pretendVulkanHeaderVersion = 123;

std::vector<GraphConstantBinding> getSegmentConstantBindings(const ModelSequenceTableDecoder &decoder,
                                                             uint32_t segmentIdx) {
    const auto *const handle = decoder.getSegmentConstantBindingsHandle(segmentIdx);
    const auto size = decoder.getGraphConstantBindingsSize(handle);
    std::vector<GraphConstantBinding> bindings;
    bindings.reserve(size);
    for (uint32_t bindingIdx = 0; bindingIdx < size; ++bindingIdx) {
        bindings.push_back(decoder.getGraphConstantBinding(handle, bindingIdx));
    }
    return bindings;
}

std::unique_ptr<ConstantDecoder> CreateConstantDecoderForVgf(const std::string &data) {
    std::unique_ptr<HeaderDecoder> headerDecoder =
        CreateHeaderDecoder(data.c_str(), static_cast<uint64_t>(HeaderSize()), static_cast<uint64_t>(data.size()));
    if (headerDecoder == nullptr) {
        return nullptr;
    }
    return CreateConstantDecoder(data.c_str() + headerDecoder->GetConstantsOffset(), headerDecoder->GetConstantsSize());
}

SegmentInfoRef AddSegmentInfoWithLegacyConstants(Encoder &encoder, ModuleRef module, const std::string &name,
                                                 const std::vector<DescriptorSetInfoRef> &descriptors,
                                                 const std::vector<BindingSlotRef> &inputs,
                                                 const std::vector<BindingSlotRef> &outputs,
                                                 const std::vector<ConstantRef> &constants,
                                                 const std::array<uint32_t, 3> &dispatchShape = {},
                                                 const std::vector<PushConstRangeRef> &pushConstRanges = {}) {
#if defined(__GNUC__) || defined(__clang__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4996)
#endif
    const auto segment =
        encoder.AddSegmentInfo(module, name, descriptors, inputs, outputs, constants, dispatchShape, pushConstRanges);
#if defined(__GNUC__) || defined(__clang__)
#    pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#    pragma warning(pop)
#endif
    return segment;
}

} // namespace

TEST(CppModelSequenceTable, SegmentInfo) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddModule(ModuleType::GRAPH, "test_module", "entry_point");

    SegmentInfoRef segment =
        encoder->AddSegmentInfo(module, "test_segment", {}, {}, {}, std::vector<GraphConstantBindingRef>{});

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder =
        CreateHeaderDecoder(data.c_str(), static_cast<uint64_t>(HeaderSize()), static_cast<uint64_t>(data.size()));
    ASSERT_NE(headerDecoder, nullptr);

    std::unique_ptr<ModelSequenceTableDecoder> decoder = CreateModelSequenceTableDecoder(
        data.c_str() + headerDecoder->GetModelSequenceTableOffset(), headerDecoder->GetModelSequenceTableSize());
    ASSERT_NE(decoder, nullptr);

    ASSERT_TRUE(decoder->modelSequenceTableSize() == 1);
    ASSERT_TRUE(decoder->getSegmentType(segment.reference) == ModuleType::GRAPH);
    ASSERT_TRUE(decoder->getSegmentName(segment.reference) == std::string("test_segment"));
    ASSERT_TRUE(decoder->getSegmentModuleIndex(segment.reference) == module.reference);
}

TEST(CppModelSequenceTable, DescripterSetInfo) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddModule(ModuleType::GRAPH, "test_module", "entry_point");

    DescriptorSetInfoRef descriptor = encoder->AddDescriptorSetInfo({}, 7);
    std::vector<DescriptorSetInfoRef> descriptors = {descriptor};

    SegmentInfoRef segment =
        encoder->AddSegmentInfo(module, "test_segment", descriptors, {}, {}, std::vector<GraphConstantBindingRef>{});

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder =
        CreateHeaderDecoder(data.c_str(), static_cast<uint64_t>(HeaderSize()), static_cast<uint64_t>(data.size()));
    ASSERT_NE(headerDecoder, nullptr);

    std::unique_ptr<ModelSequenceTableDecoder> decoder = CreateModelSequenceTableDecoder(
        data.c_str() + headerDecoder->GetModelSequenceTableOffset(), headerDecoder->GetModelSequenceTableSize());
    ASSERT_NE(decoder, nullptr);

    ASSERT_TRUE(decoder->modelSequenceTableSize() == 1);
    ASSERT_TRUE(decoder->getSegmentDescriptorSetInfosSize(segment.reference) == 1);
    ASSERT_TRUE(decoder->getSegmentDescriptorSetIndex(segment.reference, 0) == 7);
}

TEST(CppModelSequenceTable, DescripterSetInfoLegacyFallback) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddModule(ModuleType::GRAPH, "test_module", "entry_point");

    DescriptorSetInfoRef descriptor0 = encoder->AddDescriptorSetInfo();
    DescriptorSetInfoRef descriptor1 = encoder->AddDescriptorSetInfo();
    std::vector<DescriptorSetInfoRef> descriptors = {descriptor0, descriptor1};

    SegmentInfoRef segment =
        encoder->AddSegmentInfo(module, "test_segment", descriptors, {}, {}, std::vector<GraphConstantBindingRef>{});

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder =
        CreateHeaderDecoder(data.c_str(), static_cast<uint64_t>(HeaderSize()), static_cast<uint64_t>(data.size()));
    ASSERT_NE(headerDecoder, nullptr);

    std::unique_ptr<ModelSequenceTableDecoder> decoder = CreateModelSequenceTableDecoder(
        data.c_str() + headerDecoder->GetModelSequenceTableOffset(), headerDecoder->GetModelSequenceTableSize());
    ASSERT_NE(decoder, nullptr);

    ASSERT_TRUE(decoder->modelSequenceTableSize() == 1);
    ASSERT_TRUE(decoder->getSegmentDescriptorSetInfosSize(segment.reference) == 2);
    ASSERT_TRUE(decoder->getSegmentDescriptorSetIndex(segment.reference, 0) == 0);
    ASSERT_TRUE(decoder->getSegmentDescriptorSetIndex(segment.reference, 1) == 1);
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

    DescriptorSetInfoRef descriptor = encoder->AddDescriptorSetInfo(bindings, 3);
    std::vector<DescriptorSetInfoRef> descriptors = {descriptor};

    SegmentInfoRef segment =
        encoder->AddSegmentInfo(module, "test_segment", descriptors, {}, {}, std::vector<GraphConstantBindingRef>{});

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string vgfData = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(
        vgfData.c_str(), static_cast<uint64_t>(HeaderSize()), static_cast<uint64_t>(vgfData.size()));
    ASSERT_NE(headerDecoder, nullptr);

    //! [ModelSequenceTableDecodingSample0 begin]
    std::unique_ptr<ModelSequenceTableDecoder> seqTableDecoder = CreateModelSequenceTableDecoder(
        vgfData.c_str() + headerDecoder->GetModelSequenceTableOffset(), headerDecoder->GetModelSequenceTableSize());
    ASSERT_NE(seqTableDecoder, nullptr);
    //! [ModelSequenceTableDecodingSample0 end]

    ASSERT_TRUE(seqTableDecoder->modelSequenceTableSize() == 1);
    ASSERT_TRUE(seqTableDecoder->getSegmentDescriptorSetInfosSize(segment.reference) == 1);
    ASSERT_TRUE(seqTableDecoder->getSegmentDescriptorSetIndex(segment.reference, 0) == 3);

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
    ModuleRef module = encoder->AddModule(ModuleType::GRAPH, "test_module", "entry_point");
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

    DescriptorSetInfoRef inputDescriptorRef = encoder->AddDescriptorSetInfo(inputBindings, 4);
    DescriptorSetInfoRef outputDescriptorRef = encoder->AddDescriptorSetInfo(outputBindings, 9);

    std::vector<DescriptorSetInfoRef> descriptorRefs = {inputDescriptorRef, outputDescriptorRef};
    //! [BindingSlotEncodingSample1 end]

    //! [BindingSlotEncodingSample3 begin]
    SegmentInfoRef segment = encoder->AddSegmentInfo(module, "test_segment", descriptorRefs, inputBindings,
                                                     outputBindings, std::vector<GraphConstantBindingRef>{});
    //! [BindingSlotEncodingSample3 end]

    //! [BindingSlotEncodingSample4 begin]
    encoder->AddModelSequenceInputsOutputs(inputBindings, {"input"}, outputBindings, {"output"});
    //! [BindingSlotEncodingSample4 end]

    //! [BindingSlotEncodingSample5 begin]
    std::stringstream buffer;
    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));
    //! [BindingSlotEncodingSample5 end]

    std::string data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder =
        CreateHeaderDecoder(data.c_str(), static_cast<uint64_t>(HeaderSize()), static_cast<uint64_t>(data.size()));
    ASSERT_NE(headerDecoder, nullptr);

    std::unique_ptr<ModelSequenceTableDecoder> seqTableDecoder = CreateModelSequenceTableDecoder(
        data.c_str() + headerDecoder->GetModelSequenceTableOffset(), headerDecoder->GetModelSequenceTableSize());
    ASSERT_NE(seqTableDecoder, nullptr);

    ASSERT_TRUE(seqTableDecoder->modelSequenceTableSize() == 1);
    ASSERT_TRUE(seqTableDecoder->getSegmentDescriptorSetIndex(segment.reference, 0) == 4);
    ASSERT_TRUE(seqTableDecoder->getSegmentDescriptorSetIndex(segment.reference, 1) == 9);

    BindingSlotArrayHandle bindingSlotsHandle = seqTableDecoder->getSegmentInputBindingSlotsHandle(segment.reference);

    ASSERT_TRUE(seqTableDecoder->getBindingsSize(bindingSlotsHandle) == 1);
    ASSERT_TRUE(seqTableDecoder->getBindingsSize(bindingSlotsHandle) == 1);
    ASSERT_TRUE(seqTableDecoder->getBindingSlotBinding(bindingSlotsHandle, 0) == 0);
    ASSERT_TRUE(seqTableDecoder->getBindingSlotMrtIndex(bindingSlotsHandle, 0) == 2);

    bindingSlotsHandle = seqTableDecoder->getSegmentOutputBindingSlotsHandle(segment.reference);

    ASSERT_TRUE(seqTableDecoder->getBindingsSize(bindingSlotsHandle) == 1);
    ASSERT_TRUE(seqTableDecoder->getBindingSlotBinding(bindingSlotsHandle, 0) == 1);
    ASSERT_TRUE(seqTableDecoder->getBindingSlotMrtIndex(bindingSlotsHandle, 0) == 5);

    NameArrayHandle inputNames = seqTableDecoder->getModelSequenceInputNamesHandle();
    ASSERT_TRUE(seqTableDecoder->getNamesSize(inputNames) == 1);
    ASSERT_TRUE(seqTableDecoder->getName(inputNames, 0) == "input");

    NameArrayHandle outputNames = seqTableDecoder->getModelSequenceOutputNamesHandle();
    ASSERT_TRUE(seqTableDecoder->getNamesSize(outputNames) == 1);
    ASSERT_TRUE(seqTableDecoder->getName(outputNames, 0) == "output");
}

TEST(CppModelSequenceTable, BindingSlot) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddModule(ModuleType::GRAPH, "test_module", "entry_point");

    BindingSlotRef inputBinding = encoder->AddBindingSlot(1, ResourceRef{2});
    std::vector<BindingSlotRef> inputBindings = {inputBinding};

    BindingSlotRef outputBinding = encoder->AddBindingSlot(4, ResourceRef{5});
    std::vector<BindingSlotRef> outputBindings = {outputBinding};

    encoder->AddModelSequenceInputsOutputs(inputBindings, {"input_0"}, outputBindings, {"output_0"});

    encoder->AddSegmentInfo(module, "test_segment", {}, inputBindings, outputBindings,
                            std::vector<GraphConstantBindingRef>{});

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder =
        CreateHeaderDecoder(data.c_str(), static_cast<uint64_t>(HeaderSize()), static_cast<uint64_t>(data.size()));
    ASSERT_NE(headerDecoder, nullptr);

    std::unique_ptr<ModelSequenceTableDecoder> seqTableDecoder = CreateModelSequenceTableDecoder(
        data.c_str() + headerDecoder->GetModelSequenceTableOffset(), headerDecoder->GetModelSequenceTableSize());
    ASSERT_NE(seqTableDecoder, nullptr);

    ASSERT_TRUE(seqTableDecoder->modelSequenceTableSize() == 1);

    BindingSlotArrayHandle inputsHandle = seqTableDecoder->getModelSequenceInputBindingSlotsHandle();
    ASSERT_TRUE(seqTableDecoder->getBindingsSize(inputsHandle) == 1);
    ASSERT_TRUE(seqTableDecoder->getBindingSlotBinding(inputsHandle, 0) == 1);
    ASSERT_TRUE(seqTableDecoder->getBindingSlotMrtIndex(inputsHandle, 0) == 2);

    BindingSlotArrayHandle outputsHandle = seqTableDecoder->getModelSequenceOutputBindingSlotsHandle();
    ASSERT_TRUE(seqTableDecoder->getBindingsSize(outputsHandle) == 1);
    ASSERT_TRUE(seqTableDecoder->getBindingSlotBinding(outputsHandle, 0) == 4);
    ASSERT_TRUE(seqTableDecoder->getBindingSlotMrtIndex(outputsHandle, 0) == 5);

    NameArrayHandle inputNames = seqTableDecoder->getModelSequenceInputNamesHandle();
    ASSERT_TRUE(seqTableDecoder->getNamesSize(inputNames) == 1);
    ASSERT_TRUE(seqTableDecoder->getName(inputNames, 0) == "input_0");

    NameArrayHandle outputNames = seqTableDecoder->getModelSequenceOutputNamesHandle();
    ASSERT_TRUE(seqTableDecoder->getNamesSize(outputNames) == 1);
    ASSERT_TRUE(seqTableDecoder->getName(outputNames, 0) == "output_0");
}

TEST(CppModelSequenceTable, SegmentConstants) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddModule(ModuleType::GRAPH, "test_module", "entry_point");

    std::vector<ConstantRef> constants = {{1}, {2}, {3}};

    SegmentInfoRef segment = AddSegmentInfoWithLegacyConstants(*encoder, module, "test_segment", {}, {}, {}, constants);

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder =
        CreateHeaderDecoder(data.c_str(), static_cast<uint64_t>(HeaderSize()), static_cast<uint64_t>(data.size()));
    ASSERT_NE(headerDecoder, nullptr);

    std::unique_ptr<ModelSequenceTableDecoder> decoder = CreateModelSequenceTableDecoder(
        data.c_str() + headerDecoder->GetModelSequenceTableOffset(), headerDecoder->GetModelSequenceTableSize());
    ASSERT_NE(decoder, nullptr);

    ASSERT_TRUE(decoder->modelSequenceTableSize() == 1);

    DataView<uint32_t> constantIndexes = decoder->getSegmentConstantIndexes(segment.reference);
    ASSERT_FALSE(constantIndexes.empty());
    ASSERT_TRUE(constantIndexes.size() == 3);
    ASSERT_TRUE(constantIndexes[0] == constants[0].reference);
    ASSERT_TRUE(constantIndexes[1] == constants[1].reference);
    ASSERT_TRUE(constantIndexes[2] == constants[2].reference);
}

TEST(CppModelSequenceTable, SegmentConstantBindings) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddModule(ModuleType::GRAPH, "test_module", "entry_point");

    const std::array<uint8_t, 2> unusedData = {'u', 'n'};
    const std::array<uint8_t, 2> firstBoundData = {'a', 'b'};
    const std::array<uint8_t, 2> secondBoundData = {'c', 'd'};
    encoder->AddConstant(ResourceRef{0}, unusedData.data(), unusedData.size());
    ConstantRef firstBoundConstant = encoder->AddConstant(ResourceRef{1}, firstBoundData.data(), firstBoundData.size());
    ConstantRef secondBoundConstant =
        encoder->AddConstant(ResourceRef{2}, secondBoundData.data(), secondBoundData.size());

    std::vector<GraphConstantBindingRef> constantBindings = {{10000, firstBoundConstant}, {7, secondBoundConstant}};

    SegmentInfoRef segment = encoder->AddSegmentInfo(module, "test_segment", {}, {}, {}, constantBindings);

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder =
        CreateHeaderDecoder(data.c_str(), static_cast<uint64_t>(HeaderSize()), static_cast<uint64_t>(data.size()));
    ASSERT_NE(headerDecoder, nullptr);

    std::unique_ptr<ModelSequenceTableDecoder> decoder = CreateModelSequenceTableDecoder(
        data.c_str() + headerDecoder->GetModelSequenceTableOffset(), headerDecoder->GetModelSequenceTableSize());
    ASSERT_NE(decoder, nullptr);

    DataView<uint32_t> constantIndexes = decoder->getSegmentConstantIndexes(segment.reference);
    ASSERT_EQ(constantIndexes.size(), 2);
    EXPECT_EQ(constantIndexes[0], firstBoundConstant.reference);
    EXPECT_EQ(constantIndexes[1], secondBoundConstant.reference);

    std::vector<GraphConstantBinding> decodedBindings = getSegmentConstantBindings(*decoder, segment.reference);
    ASSERT_EQ(decodedBindings.size(), 2);
    EXPECT_EQ(decodedBindings[0].graphConstantId, 10000);
    EXPECT_EQ(decodedBindings[0].constantIndex, firstBoundConstant.reference);
    EXPECT_EQ(decodedBindings[1].graphConstantId, 7);
    EXPECT_EQ(decodedBindings[1].constantIndex, secondBoundConstant.reference);

    std::unique_ptr<ConstantDecoder> constantDecoder = CreateConstantDecoderForVgf(data);
    ASSERT_NE(constantDecoder, nullptr);
    EXPECT_EQ(constantDecoder->getConstant(decodedBindings[0].constantIndex),
              DataView<uint8_t>(firstBoundData.data(), firstBoundData.size()));
    EXPECT_EQ(constantDecoder->getConstant(decodedBindings[1].constantIndex),
              DataView<uint8_t>(secondBoundData.data(), secondBoundData.size()));
}

TEST(CppModelSequenceTable, OverlappingGraphConstantIdsUseSegmentLocalBindings) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module0 = encoder->AddModule(ModuleType::GRAPH, "test_module_0", "entry_point");
    ModuleRef module1 = encoder->AddModule(ModuleType::GRAPH, "test_module_1", "entry_point");
    const std::array<uint8_t, 2> segment0Data = {'a', 'b'};
    const std::array<uint8_t, 2> segment1Data = {'c', 'd'};
    ConstantRef segment0Constant = encoder->AddConstant(ResourceRef{0}, segment0Data.data(), segment0Data.size());
    ConstantRef segment1Constant = encoder->AddConstant(ResourceRef{1}, segment1Data.data(), segment1Data.size());

    SegmentInfoRef segment0 =
        encoder->AddSegmentInfo(module0, "test_segment_0", {}, {}, {}, {GraphConstantBindingRef{0, segment0Constant}});
    SegmentInfoRef segment1 =
        encoder->AddSegmentInfo(module1, "test_segment_1", {}, {}, {}, {GraphConstantBindingRef{0, segment1Constant}});

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder =
        CreateHeaderDecoder(data.c_str(), static_cast<uint64_t>(HeaderSize()), static_cast<uint64_t>(data.size()));
    ASSERT_NE(headerDecoder, nullptr);

    std::unique_ptr<ModelSequenceTableDecoder> decoder = CreateModelSequenceTableDecoder(
        data.c_str() + headerDecoder->GetModelSequenceTableOffset(), headerDecoder->GetModelSequenceTableSize());
    ASSERT_NE(decoder, nullptr);

    std::vector<GraphConstantBinding> segment0Bindings = getSegmentConstantBindings(*decoder, segment0.reference);
    ASSERT_EQ(segment0Bindings.size(), 1);
    EXPECT_EQ(segment0Bindings[0].graphConstantId, 0);
    EXPECT_EQ(segment0Bindings[0].constantIndex, segment0Constant.reference);

    std::vector<GraphConstantBinding> segment1Bindings = getSegmentConstantBindings(*decoder, segment1.reference);
    ASSERT_EQ(segment1Bindings.size(), 1);
    EXPECT_EQ(segment1Bindings[0].graphConstantId, 0);
    EXPECT_EQ(segment1Bindings[0].constantIndex, segment1Constant.reference);

    std::unique_ptr<ConstantDecoder> constantDecoder = CreateConstantDecoderForVgf(data);
    ASSERT_NE(constantDecoder, nullptr);
    EXPECT_EQ(constantDecoder->getConstant(segment0Bindings[0].constantIndex),
              DataView<uint8_t>(segment0Data.data(), segment0Data.size()));
    EXPECT_EQ(constantDecoder->getConstant(segment1Bindings[0].constantIndex),
              DataView<uint8_t>(segment1Data.data(), segment1Data.size()));
}

TEST(CppModelSequenceTable, LegacySegmentConstantsDecodeAsIdentityBindings) {
    flatbuffers::FlatBufferBuilder builder;

    const std::vector<uint32_t> constants = {1, 2, 3};
    const std::vector<flatbuffers::Offset<VGF::DescriptorSetInfo>> descriptors;
    const std::vector<flatbuffers::Offset<VGF::BindingSlot>> inputs;
    const std::vector<flatbuffers::Offset<VGF::BindingSlot>> outputs;
    const auto segment = VGF::CreateSegmentInfoDirect(builder, VGF::ModuleType_GRAPH, "test_segment", 0, &descriptors,
                                                      &inputs, &outputs, &constants);
    const std::vector<flatbuffers::Offset<VGF::SegmentInfo>> segments = {segment};
    const auto modelSequence = VGF::CreateModelSequenceTableDirect(builder, &segments);
    builder.Finish(modelSequence);

    std::unique_ptr<ModelSequenceTableDecoder> decoder =
        CreateModelSequenceTableDecoder(builder.GetBufferPointer(), builder.GetSize());
    ASSERT_NE(decoder, nullptr);

    std::vector<GraphConstantBinding> decodedBindings = getSegmentConstantBindings(*decoder, 0);
    ASSERT_EQ(decodedBindings.size(), constants.size());
    for (size_t idx = 0; idx < constants.size(); ++idx) {
        EXPECT_EQ(decodedBindings[idx].graphConstantId, constants[idx]);
        EXPECT_EQ(decodedBindings[idx].constantIndex, constants[idx]);
    }
}

TEST(CppModelSequenceTable, SegmentDispatchShape) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddModule(ModuleType::GRAPH, "test_module", "entry_point");

    std::array<uint32_t, 3> dispatchShape = {1, 2, 3};

    SegmentInfoRef segment = encoder->AddSegmentInfo(module, "test_segment", {}, {}, {},
                                                     std::vector<GraphConstantBindingRef>{}, dispatchShape);

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder =
        CreateHeaderDecoder(data.c_str(), static_cast<uint64_t>(HeaderSize()), static_cast<uint64_t>(data.size()));
    ASSERT_NE(headerDecoder, nullptr);

    std::unique_ptr<ModelSequenceTableDecoder> decoder = CreateModelSequenceTableDecoder(
        data.c_str() + headerDecoder->GetModelSequenceTableOffset(), headerDecoder->GetModelSequenceTableSize());
    ASSERT_NE(decoder, nullptr);

    ASSERT_TRUE(decoder->modelSequenceTableSize() == 1);

    DataView<uint32_t> checkShape = decoder->getSegmentDispatchShape(segment.reference);
    ASSERT_FALSE(checkShape.empty());
    ASSERT_TRUE(checkShape.size() == 3);
    ASSERT_TRUE(checkShape[0] == dispatchShape[0]);
    ASSERT_TRUE(checkShape[1] == dispatchShape[1]);
    ASSERT_TRUE(checkShape[2] == dispatchShape[2]);
}

TEST(CppModelSequenceTable, PushConstantRange) {
    std::stringstream buffer;

    std::unique_ptr<Encoder> encoder = CreateEncoder(pretendVulkanHeaderVersion);
    ModuleRef module = encoder->AddModule(ModuleType::GRAPH, "test_module", "entry_point");

    PushConstRangeRef pushConstRange = encoder->AddPushConstRange(1, 2, 3);
    std::vector<PushConstRangeRef> pushConstRanges = {pushConstRange};

    SegmentInfoRef segment = encoder->AddSegmentInfo(module, "test_segment", {}, {}, {},
                                                     std::vector<GraphConstantBindingRef>{}, {}, pushConstRanges);

    encoder->Finish();
    ASSERT_TRUE(encoder->WriteTo(buffer));

    std::string data = buffer.str();

    std::unique_ptr<HeaderDecoder> headerDecoder =
        CreateHeaderDecoder(data.c_str(), static_cast<uint64_t>(HeaderSize()), static_cast<uint64_t>(data.size()));
    ASSERT_NE(headerDecoder, nullptr);

    std::unique_ptr<ModelSequenceTableDecoder> seqTableDecoder = CreateModelSequenceTableDecoder(
        data.c_str() + headerDecoder->GetModelSequenceTableOffset(), headerDecoder->GetModelSequenceTableSize());
    ASSERT_NE(seqTableDecoder, nullptr);

    ASSERT_TRUE(seqTableDecoder->modelSequenceTableSize() == 1);

    PushConstantRangeHandle handle = seqTableDecoder->getSegmentPushConstRange(segment.reference);

    ASSERT_TRUE(seqTableDecoder->getPushConstRangesSize(handle) == 1);
    ASSERT_TRUE(seqTableDecoder->getPushConstRangeStageFlags(handle, pushConstRange.reference) == 1);
    ASSERT_TRUE(seqTableDecoder->getPushConstRangeOffset(handle, pushConstRange.reference) == 2);
    ASSERT_TRUE(seqTableDecoder->getPushConstRangeSize(handle, pushConstRange.reference) == 3);
}

TEST(CppVerify, ModelSequenceSizeWrapRejected) {
    Logger logger;
    const uint64_t sequenceOffset = 32;
    const auto sequenceSize = static_cast<uint64_t>(SIZE_MAX_VALUE);
    const size_t fileSize = 131;

    std::vector<uint8_t> buffer(fileSize, 0);
    Header header({0, 0}, {sequenceOffset, sequenceSize}, {0, 0}, {0, 0}, pretendVulkanHeaderVersion);
    std::memcpy(buffer.data(), &header, sizeof(Header));

    EXPECT_EQ(nullptr, CreateHeaderDecoder(buffer.data(), static_cast<uint64_t>(HeaderSize()),
                                           static_cast<uint64_t>(buffer.size())));
    EXPECT_TRUE(logger.contains({"section bounds invalid"}));
    EXPECT_EQ(nullptr, CreateModelSequenceTableDecoder(buffer.data() + sequenceOffset, sequenceSize));
    EXPECT_TRUE(logger.contains({"VerifyModelSequenceTable", "size out of bounds"}));
}

TEST(CppVerify, ModelSequenceTooSmallRejected) {
    Logger logger;
    std::array<uint8_t, 2> buffer{0, 0};

    EXPECT_EQ(nullptr, CreateModelSequenceTableDecoder(buffer.data(), buffer.size()));
    EXPECT_TRUE(logger.contains({"VerifyModelSequenceTable", "size smaller than header"}));
}

TEST(CppVerify, ModelSequenceMisalignedRejected) {
    Logger logger;
    const uint64_t sequenceOffset = 129;
    const uint64_t sequenceSize = 32;
    const size_t fileSize = 177;

    std::vector<uint8_t> buffer(fileSize, 0);
    Header header({0, 0}, {sequenceOffset, sequenceSize}, {0, 0}, {0, 0}, pretendVulkanHeaderVersion);
    std::memcpy(buffer.data(), &header, sizeof(Header));

    EXPECT_EQ(nullptr, CreateHeaderDecoder(buffer.data(), static_cast<uint64_t>(HeaderSize()),
                                           static_cast<uint64_t>(buffer.size())));
    EXPECT_EQ(nullptr, CreateModelSequenceTableDecoder(buffer.data() + sequenceOffset, sequenceSize));
    EXPECT_TRUE(logger.contains({"VerifyModelSequenceTable", "data alignment invalid"}));
}

TEST(CppVerify, ModelSequenceFlatbufferVerifyRejected) {
    Logger logger;
    std::array<uint8_t, 32> buffer{};
    buffer.fill(0xFF);

    EXPECT_EQ(nullptr, CreateModelSequenceTableDecoder(buffer.data(), buffer.size()));
    EXPECT_TRUE(logger.contains({"VerifyModelSequenceTable", "verification failed"}));
}

TEST(CModelSequenceTable, SegmentInfo) {
    mlsdk_encoder *encoder = mlsdk_encoder_create(pretendVulkanHeaderVersion);
    mlsdk_encoder_module_ref module = mlsdk_encoder_add_spirv_module(encoder, mlsdk_encoder_module_type_graph,
                                                                     "test_module", "entry_point", nullptr, 0);

    mlsdk_encoder_segment_info_ref segment = mlsdk_encoder_add_segment_info(
        encoder, module, "test_segment", nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, nullptr, 0);

    std::string data = testutils::FinishAndWriteCEncoder(encoder);

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
    ASSERT_TRUE(moduleSection.size > 0);
    ASSERT_TRUE(moduleSection.offset == HEADER_HEADER_SIZE_VALUE);

    std::vector<uint8_t> moduleTableDecoderMemory;
    moduleTableDecoderMemory.resize(mlsdk_decoder_module_table_decoder_mem_reqs());
    mlsdk_decoder_module_table_decoder *moduleTabledecoder = mlsdk_decoder_create_module_table_decoder(
        data.c_str() + moduleSection.offset, moduleSection.size, moduleTableDecoderMemory.data());
    ASSERT_NE(moduleTabledecoder, nullptr);

    ASSERT_TRUE(mlsdk_decoder_get_module_table_num_entries(moduleTabledecoder) == 1);

    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    ASSERT_TRUE(modelSequenceSection.size > 0);
    ASSERT_TRUE(modelSequenceSection.offset ==
                testutils::AlignUp(HEADER_HEADER_SIZE_VALUE + moduleSection.size, VGF_SECTION_ALIGNMENT_VALUE));
    std::vector<uint8_t> modelSequenceDecoderMemory;
    modelSequenceDecoderMemory.resize(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    mlsdk_decoder_model_sequence_decoder *modelSequenceDecoder = mlsdk_decoder_create_model_sequence_decoder(
        data.c_str() + modelSequenceSection.offset, modelSequenceSection.size, modelSequenceDecoderMemory.data());
    ASSERT_NE(modelSequenceDecoder, nullptr);

    ASSERT_TRUE(mlsdk_decoder_get_model_sequence_table_size(modelSequenceDecoder) == 1);
    ASSERT_TRUE(mlsdk_decoder_model_sequence_get_segment_type(modelSequenceDecoder, segment.reference) ==
                mlsdk_decoder_module_type_graph);
    ASSERT_TRUE(strcmp(mlsdk_decoder_model_sequence_get_segment_name(modelSequenceDecoder, segment.reference),
                       "test_segment") == 0);
    ASSERT_TRUE(mlsdk_decoder_model_sequence_get_segment_module_index(modelSequenceDecoder, segment.reference) ==
                module.reference);
}

TEST(CModelSequenceTable, DescripterSetInfo) {
    mlsdk_encoder *encoder = mlsdk_encoder_create(pretendVulkanHeaderVersion);
    mlsdk_encoder_module_ref module = mlsdk_encoder_add_spirv_module(encoder, mlsdk_encoder_module_type_graph,
                                                                     "test_module", "entry_point", nullptr, 0);

    mlsdk_encoder_descriptor_set_info_ref descriptor = mlsdk_encoder_add_descriptor_set_info(encoder, nullptr, 0, 11);
    std::vector<mlsdk_encoder_descriptor_set_info_ref> descriptors = {descriptor};

    mlsdk_encoder_segment_info_ref segment =
        mlsdk_encoder_add_segment_info(encoder, module, "test_segment", descriptors.data(), descriptors.size(), nullptr,
                                       0, nullptr, 0, nullptr, 0, nullptr, nullptr, 0);

    std::string data = testutils::FinishAndWriteCEncoder(encoder);
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
    ASSERT_TRUE(moduleSection.size > 0);
    ASSERT_TRUE(moduleSection.offset == HEADER_HEADER_SIZE_VALUE);

    std::vector<uint8_t> moduleTableDecoderMemory;
    moduleTableDecoderMemory.resize(mlsdk_decoder_module_table_decoder_mem_reqs());
    mlsdk_decoder_module_table_decoder *moduleTabledecoder = mlsdk_decoder_create_module_table_decoder(
        data.c_str() + moduleSection.offset, moduleSection.size, moduleTableDecoderMemory.data());
    ASSERT_NE(moduleTabledecoder, nullptr);

    ASSERT_TRUE(mlsdk_decoder_get_module_table_num_entries(moduleTabledecoder) == 1);

    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    ASSERT_TRUE(modelSequenceSection.size > 0);
    ASSERT_TRUE(modelSequenceSection.offset ==
                testutils::AlignUp(HEADER_HEADER_SIZE_VALUE + moduleSection.size, VGF_SECTION_ALIGNMENT_VALUE));
    std::vector<uint8_t> modelSequenceDecoderMemory;
    modelSequenceDecoderMemory.resize(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    mlsdk_decoder_model_sequence_decoder *modelSequenceDecoder = mlsdk_decoder_create_model_sequence_decoder(
        data.c_str() + modelSequenceSection.offset, modelSequenceSection.size, modelSequenceDecoderMemory.data());
    ASSERT_NE(modelSequenceDecoder, nullptr);

    ASSERT_TRUE(mlsdk_decoder_get_model_sequence_table_size(modelSequenceDecoder) == 1);
    ASSERT_TRUE(
        mlsdk_decoder_model_sequence_get_segment_descriptorset_info_size(modelSequenceDecoder, segment.reference) == 1);
    ASSERT_TRUE(
        mlsdk_decoder_model_sequence_get_segment_descriptorset_index(modelSequenceDecoder, segment.reference, 0) == 11);
}

TEST(CModelSequenceTable, DescriptorBindingSlot) {
    mlsdk_encoder *encoder = mlsdk_encoder_create(pretendVulkanHeaderVersion);
    mlsdk_encoder_module_ref module = mlsdk_encoder_add_spirv_module(encoder, mlsdk_encoder_module_type_graph,
                                                                     "test_module", "entry_point", nullptr, 0);

    mlsdk_encoder_binding_slot_ref binding = mlsdk_encoder_add_binding_slot(encoder, 1, {2});
    std::vector<mlsdk_encoder_binding_slot_ref> bindings = {binding};

    mlsdk_encoder_descriptor_set_info_ref descriptor =
        mlsdk_encoder_add_descriptor_set_info(encoder, bindings.data(), bindings.size(), 5);
    std::vector<mlsdk_encoder_descriptor_set_info_ref> descriptors = {descriptor};

    mlsdk_encoder_segment_info_ref segment =
        mlsdk_encoder_add_segment_info(encoder, module, "test_segment", descriptors.data(), descriptors.size(), nullptr,
                                       0, nullptr, 0, nullptr, 0, nullptr, nullptr, 0);

    std::string data = testutils::FinishAndWriteCEncoder(encoder);
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
    ASSERT_TRUE(moduleSection.size > 0);
    ASSERT_TRUE(moduleSection.offset == HEADER_HEADER_SIZE_VALUE);

    std::vector<uint8_t> moduleTableDecoderMemory;
    moduleTableDecoderMemory.resize(mlsdk_decoder_module_table_decoder_mem_reqs());
    mlsdk_decoder_module_table_decoder *moduleTabledecoder = mlsdk_decoder_create_module_table_decoder(
        data.c_str() + moduleSection.offset, moduleSection.size, moduleTableDecoderMemory.data());
    ASSERT_NE(moduleTabledecoder, nullptr);

    ASSERT_TRUE(mlsdk_decoder_get_module_table_num_entries(moduleTabledecoder) == 1);

    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    ASSERT_TRUE(modelSequenceSection.size > 0);
    ASSERT_TRUE(modelSequenceSection.offset ==
                testutils::AlignUp(HEADER_HEADER_SIZE_VALUE + moduleSection.size, VGF_SECTION_ALIGNMENT_VALUE));

    std::vector<uint8_t> modelSequenceDecoderMemory;
    modelSequenceDecoderMemory.resize(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    mlsdk_decoder_model_sequence_decoder *modelSequenceDecoder = mlsdk_decoder_create_model_sequence_decoder(
        data.c_str() + modelSequenceSection.offset, modelSequenceSection.size, modelSequenceDecoderMemory.data());
    ASSERT_NE(modelSequenceDecoder, nullptr);

    ASSERT_TRUE(mlsdk_decoder_get_model_sequence_table_size(modelSequenceDecoder) == 1);
    ASSERT_TRUE(
        mlsdk_decoder_model_sequence_get_segment_descriptorset_info_size(modelSequenceDecoder, segment.reference) == 1);
    ASSERT_TRUE(
        mlsdk_decoder_model_sequence_get_segment_descriptorset_index(modelSequenceDecoder, segment.reference, 0) == 5);

    mlsdk_decoder_binding_slots_handle handle = mlsdk_decoder_model_sequence_get_segment_descriptor_binding_slot(
        modelSequenceDecoder, segment.reference, descriptor.reference);

    ASSERT_TRUE(mlsdk_decoder_binding_slot_size(modelSequenceDecoder, handle) == 1);
    ASSERT_TRUE(mlsdk_decoder_binding_slot_binding_id(modelSequenceDecoder, handle, binding.reference) == 1);
    ASSERT_TRUE(mlsdk_decoder_binding_slot_mrt_index(modelSequenceDecoder, handle, binding.reference) == 2);
}

TEST(CModelSequenceTable, SegmentBindingSlot) {
    mlsdk_encoder *encoder = mlsdk_encoder_create(pretendVulkanHeaderVersion);
    mlsdk_encoder_module_ref module = mlsdk_encoder_add_spirv_module(encoder, mlsdk_encoder_module_type_graph,
                                                                     "test_module", "entry_point", nullptr, 0);

    mlsdk_encoder_binding_slot_ref inputBinding = mlsdk_encoder_add_binding_slot(encoder, 1, {2});
    std::vector<mlsdk_encoder_binding_slot_ref> inputBindings = {inputBinding};

    mlsdk_encoder_binding_slot_ref outputBinding = mlsdk_encoder_add_binding_slot(encoder, 4, {5});
    std::vector<mlsdk_encoder_binding_slot_ref> outputBindings = {outputBinding};

    mlsdk_encoder_segment_info_ref segment = mlsdk_encoder_add_segment_info(
        encoder, module, "test_segment", nullptr, 0, inputBindings.data(), inputBindings.size(), outputBindings.data(),
        outputBindings.size(), nullptr, 0, nullptr, nullptr, 0);
    const char *encodedInputNames[] = {"input"};
    const char *encodedOutputNames[] = {"output"};
    mlsdk_encoder_add_model_sequence_inputs_outputs(
        encoder, inputBindings.data(), inputBindings.size(), &encodedInputNames[0], std::size(encodedInputNames),
        outputBindings.data(), outputBindings.size(), &encodedOutputNames[0], std::size(encodedOutputNames));

    std::string data = testutils::FinishAndWriteCEncoder(encoder);
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
    ASSERT_TRUE(moduleSection.size > 0);
    ASSERT_TRUE(moduleSection.offset == HEADER_HEADER_SIZE_VALUE);

    std::vector<uint8_t> moduleTableDecoderMemory;
    moduleTableDecoderMemory.resize(mlsdk_decoder_module_table_decoder_mem_reqs());
    mlsdk_decoder_module_table_decoder *moduleTabledecoder = mlsdk_decoder_create_module_table_decoder(
        data.c_str() + moduleSection.offset, moduleSection.size, moduleTableDecoderMemory.data());
    ASSERT_NE(moduleTabledecoder, nullptr);

    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    ASSERT_TRUE(modelSequenceSection.size > 0);
    ASSERT_TRUE(modelSequenceSection.offset ==
                testutils::AlignUp(HEADER_HEADER_SIZE_VALUE + moduleSection.size, VGF_SECTION_ALIGNMENT_VALUE));

    ASSERT_TRUE(mlsdk_decoder_get_module_table_num_entries(moduleTabledecoder) == 1);

    std::vector<uint8_t> modelSequenceDecoderMemory;
    modelSequenceDecoderMemory.resize(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    mlsdk_decoder_model_sequence_decoder *modelSequenceDecoder = mlsdk_decoder_create_model_sequence_decoder(
        data.c_str() + modelSequenceSection.offset, modelSequenceSection.size, modelSequenceDecoderMemory.data());
    ASSERT_NE(modelSequenceDecoder, nullptr);

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

    mlsdk_decoder_names_handle inputNames = mlsdk_decoder_model_sequence_get_input_names(modelSequenceDecoder);
    ASSERT_TRUE(mlsdk_decoder_model_sequence_get_names_size(modelSequenceDecoder, inputNames) == 1);
    ASSERT_TRUE(std::string(mlsdk_decoder_model_sequence_get_name(modelSequenceDecoder, inputNames, 0)) == "input");

    mlsdk_decoder_names_handle outputNames = mlsdk_decoder_model_sequence_get_output_names(modelSequenceDecoder);
    ASSERT_TRUE(mlsdk_decoder_model_sequence_get_names_size(modelSequenceDecoder, outputNames) == 1);
    ASSERT_TRUE(std::string(mlsdk_decoder_model_sequence_get_name(modelSequenceDecoder, outputNames, 0)) == "output");
}

TEST(CModelSequenceTable, BindingSlot) {
    mlsdk_encoder *encoder = mlsdk_encoder_create(pretendVulkanHeaderVersion);
    mlsdk_encoder_module_ref module = mlsdk_encoder_add_spirv_module(encoder, mlsdk_encoder_module_type_graph,
                                                                     "test_module", "entry_point", nullptr, 0);

    mlsdk_encoder_binding_slot_ref inputBinding = mlsdk_encoder_add_binding_slot(encoder, 1, {2});
    std::vector<mlsdk_encoder_binding_slot_ref> inputBindings = {inputBinding};

    mlsdk_encoder_binding_slot_ref outputBinding = mlsdk_encoder_add_binding_slot(encoder, 4, {5});
    std::vector<mlsdk_encoder_binding_slot_ref> outputBindings = {outputBinding};

    const char *encodedInputNames[] = {"input_0"};
    const char *encodedOutputNames[] = {"output_0"};
    mlsdk_encoder_add_model_sequence_inputs_outputs(
        encoder, inputBindings.data(), inputBindings.size(), &encodedInputNames[0], std::size(encodedInputNames),
        outputBindings.data(), outputBindings.size(), &encodedOutputNames[0], std::size(encodedOutputNames));

    mlsdk_encoder_add_segment_info(encoder, module, "test_segment", nullptr, 0, inputBindings.data(),
                                   inputBindings.size(), outputBindings.data(), outputBindings.size(), nullptr, 0,
                                   nullptr, nullptr, 0);

    std::string data = testutils::FinishAndWriteCEncoder(encoder);
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
    ASSERT_TRUE(moduleSection.size > 0);
    ASSERT_TRUE(moduleSection.offset == HEADER_HEADER_SIZE_VALUE);

    std::vector<uint8_t> moduleTableDecoderMemory;
    moduleTableDecoderMemory.resize(mlsdk_decoder_module_table_decoder_mem_reqs());
    mlsdk_decoder_module_table_decoder *moduleTabledecoder = mlsdk_decoder_create_module_table_decoder(
        data.c_str() + moduleSection.offset, moduleSection.size, moduleTableDecoderMemory.data());
    ASSERT_NE(moduleTabledecoder, nullptr);

    ASSERT_TRUE(mlsdk_decoder_get_module_table_num_entries(moduleTabledecoder) == 1);

    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    ASSERT_TRUE(modelSequenceSection.size > 0);
    ASSERT_TRUE(modelSequenceSection.offset ==
                testutils::AlignUp(HEADER_HEADER_SIZE_VALUE + moduleSection.size, VGF_SECTION_ALIGNMENT_VALUE));

    std::vector<uint8_t> modelSequenceDecoderMemory;
    modelSequenceDecoderMemory.resize(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    mlsdk_decoder_model_sequence_decoder *modelSequenceDecoder = mlsdk_decoder_create_model_sequence_decoder(
        data.c_str() + modelSequenceSection.offset, modelSequenceSection.size, modelSequenceDecoderMemory.data());
    ASSERT_NE(modelSequenceDecoder, nullptr);

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

    mlsdk_decoder_names_handle inputNames = mlsdk_decoder_model_sequence_get_input_names(modelSequenceDecoder);
    ASSERT_TRUE(mlsdk_decoder_model_sequence_get_names_size(modelSequenceDecoder, inputNames) == 1);
    ASSERT_TRUE(std::string(mlsdk_decoder_model_sequence_get_name(modelSequenceDecoder, inputNames, 0)) == "input_0");

    mlsdk_decoder_names_handle outputNames = mlsdk_decoder_model_sequence_get_output_names(modelSequenceDecoder);
    ASSERT_TRUE(mlsdk_decoder_model_sequence_get_names_size(modelSequenceDecoder, outputNames) == 1);
    ASSERT_TRUE(std::string(mlsdk_decoder_model_sequence_get_name(modelSequenceDecoder, outputNames, 0)) == "output_0");
}

TEST(CModelSequenceTable, SegmentConstants) {
    mlsdk_encoder *encoder = mlsdk_encoder_create(pretendVulkanHeaderVersion);
    mlsdk_encoder_module_ref module = mlsdk_encoder_add_spirv_module(encoder, mlsdk_encoder_module_type_graph,
                                                                     "test_module", "entry_point", nullptr, 0);

    std::vector<mlsdk_encoder_constant_ref> constants = {{1}, {2}, {3}};

    mlsdk_encoder_segment_info_ref segment =
        mlsdk_encoder_add_segment_info(encoder, module, "test_segment", nullptr, 0, nullptr, 0, nullptr, 0,
                                       constants.data(), constants.size(), nullptr, nullptr, 0);

    std::string data = testutils::FinishAndWriteCEncoder(encoder);
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
    ASSERT_TRUE(moduleSection.size > 0);
    ASSERT_TRUE(moduleSection.offset == HEADER_HEADER_SIZE_VALUE);

    std::vector<uint8_t> moduleTableDecoderMemory;
    moduleTableDecoderMemory.resize(mlsdk_decoder_module_table_decoder_mem_reqs());
    mlsdk_decoder_module_table_decoder *moduleTabledecoder = mlsdk_decoder_create_module_table_decoder(
        data.c_str() + moduleSection.offset, moduleSection.size, moduleTableDecoderMemory.data());
    ASSERT_NE(moduleTabledecoder, nullptr);

    ASSERT_TRUE(mlsdk_decoder_get_module_table_num_entries(moduleTabledecoder) == 1);

    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    ASSERT_TRUE(modelSequenceSection.size > 0);
    ASSERT_TRUE(modelSequenceSection.offset ==
                testutils::AlignUp(HEADER_HEADER_SIZE_VALUE + moduleSection.size, VGF_SECTION_ALIGNMENT_VALUE));

    std::vector<uint8_t> modelSequenceDecoderMemory;
    modelSequenceDecoderMemory.resize(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    mlsdk_decoder_model_sequence_decoder *modelSequenceDecoder = mlsdk_decoder_create_model_sequence_decoder(
        data.c_str() + modelSequenceSection.offset, modelSequenceSection.size, modelSequenceDecoderMemory.data());
    ASSERT_NE(modelSequenceDecoder, nullptr);

    ASSERT_TRUE(mlsdk_decoder_get_model_sequence_table_size(modelSequenceDecoder) == 1);

    mlsdk_decoder_constant_indexes constantIndexes = {};
    mlsdk_decoder_model_sequence_get_segment_constant_indexes(modelSequenceDecoder, segment.reference,
                                                              &constantIndexes);
    ASSERT_TRUE(constantIndexes.size == 3);
    ASSERT_TRUE(constantIndexes.data[0] == constants[0].reference);
    ASSERT_TRUE(constantIndexes.data[1] == constants[1].reference);
    ASSERT_TRUE(constantIndexes.data[2] == constants[2].reference);
}

TEST(CModelSequenceTable, SegmentConstantBindings) {
    mlsdk_encoder *encoder = mlsdk_encoder_create(pretendVulkanHeaderVersion);
    mlsdk_encoder_module_ref module = mlsdk_encoder_add_spirv_module(encoder, mlsdk_encoder_module_type_graph,
                                                                     "test_module", "entry_point", nullptr, 0);

    std::vector<mlsdk_encoder_graph_constant_binding_ref> constantBindings = {{10000, {0}}, {7, {1}}};

    mlsdk_encoder_segment_info_ref segment = mlsdk_encoder_add_segment_info_with_constant_bindings(
        encoder, module, "test_segment", nullptr, 0, nullptr, 0, nullptr, 0, constantBindings.data(),
        constantBindings.size(), nullptr, nullptr, 0);

    std::string data = testutils::FinishAndWriteCEncoder(encoder);
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
    ASSERT_TRUE(moduleSection.size > 0);

    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    ASSERT_TRUE(modelSequenceSection.size > 0);
    ASSERT_TRUE(modelSequenceSection.offset ==
                testutils::AlignUp(HEADER_HEADER_SIZE_VALUE + moduleSection.size, VGF_SECTION_ALIGNMENT_VALUE));

    std::vector<uint8_t> modelSequenceDecoderMemory;
    modelSequenceDecoderMemory.resize(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    mlsdk_decoder_model_sequence_decoder *modelSequenceDecoder = mlsdk_decoder_create_model_sequence_decoder(
        data.c_str() + modelSequenceSection.offset, modelSequenceSection.size, modelSequenceDecoderMemory.data());
    ASSERT_NE(modelSequenceDecoder, nullptr);

    mlsdk_decoder_constant_indexes constantIndexes = {};
    mlsdk_decoder_model_sequence_get_segment_constant_indexes(modelSequenceDecoder, segment.reference,
                                                              &constantIndexes);
    ASSERT_EQ(constantIndexes.size, 2);
    EXPECT_EQ(constantIndexes.data[0], 0);
    EXPECT_EQ(constantIndexes.data[1], 1);

    mlsdk_decoder_graph_constant_bindings_handle decodedBindings =
        mlsdk_decoder_model_sequence_get_segment_constant_bindings(modelSequenceDecoder, segment.reference);
    ASSERT_EQ(mlsdk_decoder_graph_constant_binding_size(modelSequenceDecoder, decodedBindings), 2);
    mlsdk_decoder_graph_constant_binding binding{};
    mlsdk_decoder_graph_constant_binding_get(modelSequenceDecoder, decodedBindings, 0, &binding);
    EXPECT_EQ(binding.graph_constant_id, 10000);
    EXPECT_EQ(binding.constant_table_index, 0);
    mlsdk_decoder_graph_constant_binding_get(modelSequenceDecoder, decodedBindings, 1, &binding);
    EXPECT_EQ(binding.graph_constant_id, 7);
    EXPECT_EQ(binding.constant_table_index, 1);
}

TEST(CModelSequenceTable, SegmentDispatchShape) {
    mlsdk_encoder *encoder = mlsdk_encoder_create(pretendVulkanHeaderVersion);
    mlsdk_encoder_module_ref module = mlsdk_encoder_add_spirv_module(encoder, mlsdk_encoder_module_type_graph,
                                                                     "test_module", "entry_point", nullptr, 0);

    std::array<uint32_t, 3> dispatchShape = {1, 2, 3};

    mlsdk_encoder_segment_info_ref segment =
        mlsdk_encoder_add_segment_info(encoder, module, "test_segment", nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0,
                                       dispatchShape.data(), nullptr, 0);

    std::string data = testutils::FinishAndWriteCEncoder(encoder);
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
    ASSERT_TRUE(moduleSection.size > 0);
    ASSERT_TRUE(moduleSection.offset == HEADER_HEADER_SIZE_VALUE);

    std::vector<uint8_t> moduleTableDecoderMemory;
    moduleTableDecoderMemory.resize(mlsdk_decoder_module_table_decoder_mem_reqs());
    mlsdk_decoder_module_table_decoder *moduleTabledecoder = mlsdk_decoder_create_module_table_decoder(
        data.c_str() + moduleSection.offset, moduleSection.size, moduleTableDecoderMemory.data());
    ASSERT_NE(moduleTabledecoder, nullptr);

    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    ASSERT_TRUE(modelSequenceSection.size > 0);
    ASSERT_TRUE(modelSequenceSection.offset ==
                testutils::AlignUp(HEADER_HEADER_SIZE_VALUE + moduleSection.size, VGF_SECTION_ALIGNMENT_VALUE));

    ASSERT_TRUE(mlsdk_decoder_get_module_table_num_entries(moduleTabledecoder) == 1);

    std::vector<uint8_t> modelSequenceDecoderMemory;
    modelSequenceDecoderMemory.resize(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    mlsdk_decoder_model_sequence_decoder *modelSequenceDecoder = mlsdk_decoder_create_model_sequence_decoder(
        data.c_str() + modelSequenceSection.offset, modelSequenceSection.size, modelSequenceDecoderMemory.data());
    ASSERT_NE(modelSequenceDecoder, nullptr);

    ASSERT_TRUE(mlsdk_decoder_get_model_sequence_table_size(modelSequenceDecoder) == 1);

    mlsdk_decoder_dispatch_shape checkShape;
    mlsdk_decoder_model_sequence_get_segment_dispatch_shape(modelSequenceDecoder, segment.reference, &checkShape);
    ASSERT_TRUE(checkShape.data[0] == dispatchShape[0]);
    ASSERT_TRUE(checkShape.data[1] == dispatchShape[1]);
    ASSERT_TRUE(checkShape.data[2] == dispatchShape[2]);
}

TEST(CModelSequenceTable, PushConstantRange) {
    mlsdk_encoder *encoder = mlsdk_encoder_create(pretendVulkanHeaderVersion);
    mlsdk_encoder_module_ref module = mlsdk_encoder_add_spirv_module(encoder, mlsdk_encoder_module_type_graph,
                                                                     "test_module", "entry_point", nullptr, 0);

    //! [PushConstRangesEncodingSample0 begin]
    mlsdk_encoder_push_const_range_ref pushConstRange = mlsdk_encoder_add_push_const_range(encoder, 1, 2, 3);
    std::vector<mlsdk_encoder_push_const_range_ref> pushConstRanges = {pushConstRange};
    //! [PushConstRangesEncodingSample0 end]

    mlsdk_encoder_segment_info_ref segment =
        mlsdk_encoder_add_segment_info(encoder, module, "test_segment", nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0,
                                       nullptr, pushConstRanges.data(), pushConstRanges.size());

    std::string data = testutils::FinishAndWriteCEncoder(encoder);
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
    ASSERT_TRUE(moduleSection.size > 0);
    ASSERT_TRUE(moduleSection.offset == HEADER_HEADER_SIZE_VALUE);

    std::vector<uint8_t> moduleTableDecoderMemory;
    moduleTableDecoderMemory.resize(mlsdk_decoder_module_table_decoder_mem_reqs());
    mlsdk_decoder_module_table_decoder *moduleTabledecoder = mlsdk_decoder_create_module_table_decoder(
        data.c_str() + moduleSection.offset, moduleSection.size, moduleTableDecoderMemory.data());
    ASSERT_NE(moduleTabledecoder, nullptr);

    ASSERT_TRUE(mlsdk_decoder_get_module_table_num_entries(moduleTabledecoder) == 1);

    mlsdk_decoder_vgf_section_info modelSequenceSection;
    mlsdk_decoder_get_header_section_info(headerDecoder, mlsdk_decoder_section_model_sequence, &modelSequenceSection);
    ASSERT_TRUE(modelSequenceSection.size > 0);
    ASSERT_TRUE(modelSequenceSection.offset ==
                testutils::AlignUp(HEADER_HEADER_SIZE_VALUE + moduleSection.size, VGF_SECTION_ALIGNMENT_VALUE));

    std::vector<uint8_t> modelSequenceDecoderMemory;
    modelSequenceDecoderMemory.resize(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    mlsdk_decoder_model_sequence_decoder *modelSequenceDecoder = mlsdk_decoder_create_model_sequence_decoder(
        data.c_str() + modelSequenceSection.offset, modelSequenceSection.size, modelSequenceDecoderMemory.data());
    ASSERT_NE(modelSequenceDecoder, nullptr);

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

TEST(CVerify, ModelSequenceSizeWrapRejected) {
    Logger logger;
    const uint64_t sequenceOffset = 32;
    const auto sequenceSize = static_cast<uint64_t>(SIZE_MAX_VALUE);
    const size_t fileSize = 131;

    std::vector<uint8_t> buffer(fileSize, 0);
    Header header({0, 0}, {sequenceOffset, sequenceSize}, {0, 0}, {0, 0}, pretendVulkanHeaderVersion);
    std::memcpy(buffer.data(), &header, sizeof(Header));

    std::vector<uint8_t> headerDecoderMemory(mlsdk_decoder_header_decoder_mem_reqs());
    EXPECT_EQ(nullptr,
              mlsdk_decoder_create_header_decoder(buffer.data(), static_cast<uint64_t>(mlsdk_decoder_header_size()),
                                                  static_cast<uint64_t>(buffer.size()), headerDecoderMemory.data()));
    EXPECT_TRUE(logger.contains({"section bounds invalid"}));
    std::vector<uint8_t> decoderMemory(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    EXPECT_EQ(nullptr, mlsdk_decoder_create_model_sequence_decoder(buffer.data() + sequenceOffset, sequenceSize,
                                                                   decoderMemory.data()));
    EXPECT_TRUE(logger.contains({"VerifyModelSequenceTable", "size out of bounds"}));
}

TEST(CVerify, ModelSequenceTooSmallRejected) {
    Logger logger;
    std::array<uint8_t, 2> buffer{0, 0};

    std::vector<uint8_t> decoderMemory(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    EXPECT_EQ(nullptr, mlsdk_decoder_create_model_sequence_decoder(buffer.data(), buffer.size(), decoderMemory.data()));
    EXPECT_TRUE(logger.contains({"VerifyModelSequenceTable", "size smaller than header"}));
}

TEST(CVerify, ModelSequenceMisalignedRejected) {
    Logger logger;
    const uint64_t sequenceOffset = 129;
    const uint64_t sequenceSize = 32;
    const size_t fileSize = 177;

    std::vector<uint8_t> buffer(fileSize, 0);
    Header header({0, 0}, {sequenceOffset, sequenceSize}, {0, 0}, {0, 0}, pretendVulkanHeaderVersion);
    std::memcpy(buffer.data(), &header, sizeof(Header));

    std::vector<uint8_t> headerDecoderMemory(mlsdk_decoder_header_decoder_mem_reqs());
    EXPECT_EQ(nullptr,
              mlsdk_decoder_create_header_decoder(buffer.data(), static_cast<uint64_t>(mlsdk_decoder_header_size()),
                                                  static_cast<uint64_t>(buffer.size()), headerDecoderMemory.data()));
    std::vector<uint8_t> decoderMem(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    EXPECT_EQ(nullptr, mlsdk_decoder_create_model_sequence_decoder(buffer.data() + sequenceOffset, sequenceSize,
                                                                   decoderMem.data()));
    EXPECT_TRUE(logger.contains({"VerifyModelSequenceTable", "data alignment invalid"}));
}

TEST(CVerify, ModelSequenceFlatbufferVerifyRejected) {
    Logger logger;
    std::array<uint8_t, 32> buffer{};
    buffer.fill(0xFF);

    std::vector<uint8_t> decoderMemory(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    EXPECT_EQ(nullptr, mlsdk_decoder_create_model_sequence_decoder(buffer.data(), buffer.size(), decoderMemory.data()));
    EXPECT_TRUE(logger.contains({"VerifyModelSequenceTable", "verification failed"}));
}
