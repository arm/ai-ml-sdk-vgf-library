#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: Copyright 2024 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
""" Tests for VGF Model Sequence. """
import io

import numpy as np
import pytest
import vgfpy as vgf

pretendVulkanHeaderVersion = 123

pytestmark = pytest.mark.model_sequence_test


def test_encode_decode_model_sequence_table_segment_info():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    module = encoder.AddPlaceholderModule(
        vgf.ModuleType.Graph, "test_module", "entry_point"
    )

    segment = encoder.AddSegmentInfo(module, "test_segment")

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()

    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer)
    assert headerDecoder.IsValid()
    assert headerDecoder.CheckVersion()

    seqTableDecoder = vgf.CreateModelSequenceTableDecoder(
        buffer[
            headerDecoder.GetModuleTableOffset() + headerDecoder.GetModuleTableSize() :
        ]
    )

    assert seqTableDecoder.modelSequenceTableSize() == 1
    assert seqTableDecoder.getSegmentType(segment.reference) == vgf.ModuleType.Graph
    assert seqTableDecoder.getSegmentName(segment.reference) == "test_segment"
    assert seqTableDecoder.getSegmentModuleIndex(segment.reference) == module.reference


def test_encode_decode_model_sequence_table_descriptor_set_info_empty():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    module = encoder.AddPlaceholderModule(
        vgf.ModuleType.Graph, "test_module", "entry_point"
    )

    descriptor = encoder.AddDescriptorSetInfo()
    descriptors = [descriptor]

    segment = encoder.AddSegmentInfo(module, "test_segment", descriptors)

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()

    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer)
    assert headerDecoder.IsValid()
    assert headerDecoder.CheckVersion()

    seqTableDecoder = vgf.CreateModelSequenceTableDecoder(
        buffer[
            headerDecoder.GetModuleTableOffset() + headerDecoder.GetModuleTableSize() :
        ]
    )

    assert seqTableDecoder.modelSequenceTableSize() == 1
    assert seqTableDecoder.getSegmentDescriptorSetInfosSize(segment.reference) == 1


def test_encode_decode_model_sequence_table_descriptor_set_info_binding_slot():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    module = encoder.AddPlaceholderModule(
        vgf.ModuleType.Graph, "test_module", "entry_point"
    )

    binding = encoder.AddBindingSlot(1, vgf.ResourceRef(2))
    bindings = [binding]

    descriptor = encoder.AddDescriptorSetInfo(bindings)
    descriptors = [descriptor]

    segment = encoder.AddSegmentInfo(module, "test_segment", descriptors)

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()

    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer)
    assert headerDecoder.IsValid()
    assert headerDecoder.CheckVersion()

    seqTableDecoder = vgf.CreateModelSequenceTableDecoder(
        buffer[headerDecoder.GetModelSequenceTableOffset() :]
    )

    assert seqTableDecoder.modelSequenceTableSize() == 1
    assert seqTableDecoder.getSegmentDescriptorSetInfosSize(segment.reference) == 1

    segmentIndex = segment.reference
    discriptorSetInfoIndex = descriptor.reference

    assert segmentIndex < seqTableDecoder.modelSequenceTableSize()
    assert discriptorSetInfoIndex < seqTableDecoder.getSegmentDescriptorSetInfosSize(
        segmentIndex
    )

    bindingSlotsHandle = seqTableDecoder.getDescriptorBindingSlotsHandle(
        segmentIndex, discriptorSetInfoIndex
    )
    numSlots = seqTableDecoder.getBindingsSize(bindingSlotsHandle)

    assert numSlots == 1

    slotIndex = binding.reference
    assert slotIndex < numSlots

    bindingId = seqTableDecoder.getBindingSlotBinding(bindingSlotsHandle, slotIndex)
    mrtIndex = seqTableDecoder.getBindingSlotMrtIndex(bindingSlotsHandle, slotIndex)

    assert bindingId == 1
    assert mrtIndex == 2


def test_encode_decode_model_sequence_table_segment_input_output_binding_slot():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    module = encoder.AddPlaceholderModule(
        vgf.ModuleType.Graph, "test_module", "entry_point"
    )

    inputBinding = encoder.AddBindingSlot(1, vgf.ResourceRef(2))
    inputBindings = [inputBinding]

    outputBinding = encoder.AddBindingSlot(4, vgf.ResourceRef(5))
    outputBindings = [outputBinding]

    segment = encoder.AddSegmentInfo(
        module, "test_segment", inputs=inputBindings, outputs=outputBindings
    )

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()

    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer)
    assert headerDecoder.IsValid()
    assert headerDecoder.CheckVersion()

    seqTableDecoder = vgf.CreateModelSequenceTableDecoder(
        buffer[
            headerDecoder.GetModuleTableOffset() + headerDecoder.GetModuleTableSize() :
        ]
    )

    assert seqTableDecoder.modelSequenceTableSize() == 1

    bindingSlotsHandle = seqTableDecoder.getSegmentInputBindingSlotsHandle(
        segment.reference
    )

    assert seqTableDecoder.getBindingsSize(bindingSlotsHandle) == 1
    assert seqTableDecoder.getBindingsSize(bindingSlotsHandle) == 1
    assert seqTableDecoder.getBindingSlotBinding(bindingSlotsHandle, 0) == 1
    assert seqTableDecoder.getBindingSlotMrtIndex(bindingSlotsHandle, 0) == 2

    bindingSlotsHandle = seqTableDecoder.getSegmentOutputBindingSlotsHandle(
        segment.reference
    )

    assert seqTableDecoder.getBindingsSize(bindingSlotsHandle) == 1
    assert seqTableDecoder.getBindingSlotBinding(bindingSlotsHandle, 0) == 4
    assert seqTableDecoder.getBindingSlotMrtIndex(bindingSlotsHandle, 0) == 5


def test_encode_decode_model_sequence_table_input_output_binding_slot():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    module = encoder.AddPlaceholderModule(
        vgf.ModuleType.Graph, "test_module", "entry_point"
    )

    inputBinding = encoder.AddBindingSlot(1, vgf.ResourceRef(2))
    inputBindings = [inputBinding]

    outputBinding = encoder.AddBindingSlot(4, vgf.ResourceRef(5))
    outputBindings = [outputBinding]

    encoder.AddModelSequenceInputsOutputs(
        inputBindings, ["input_0"], outputBindings, []
    )

    segment = encoder.AddSegmentInfo(
        module, "test_segment", inputs=inputBindings, outputs=outputBindings
    )

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()

    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer)
    assert headerDecoder.IsValid()
    assert headerDecoder.CheckVersion()

    seqTableDecoder = vgf.CreateModelSequenceTableDecoder(
        buffer[
            headerDecoder.GetModuleTableOffset() + headerDecoder.GetModuleTableSize() :
        ]
    )

    assert seqTableDecoder.modelSequenceTableSize() == 1

    inputsHandle = seqTableDecoder.getModelSequenceInputBindingSlotsHandle()
    assert seqTableDecoder.getBindingsSize(inputsHandle) == 1
    assert seqTableDecoder.getBindingSlotBinding(inputsHandle, 0) == 1
    assert seqTableDecoder.getBindingSlotMrtIndex(inputsHandle, 0) == 2

    inputNames = seqTableDecoder.getModelSequenceInputNamesHandle()
    assert seqTableDecoder.getNamesSize(inputNames) == 1
    assert seqTableDecoder.getName(inputNames, 0) == "input_0"

    outputNames = seqTableDecoder.getModelSequenceOutputNamesHandle()
    assert seqTableDecoder.getNamesSize(outputNames) == 0

    outputsHandle = seqTableDecoder.getModelSequenceOutputBindingSlotsHandle()
    assert seqTableDecoder.getBindingsSize(outputsHandle) == 1
    assert seqTableDecoder.getBindingSlotBinding(outputsHandle, 0) == 4
    assert seqTableDecoder.getBindingSlotMrtIndex(outputsHandle, 0) == 5


def test_encode_decode_model_sequence_table_segment_constants():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    module = encoder.AddPlaceholderModule(
        vgf.ModuleType.Graph, "test_module", "entry_point"
    )

    constants = [vgf.ConstantRef(1), vgf.ConstantRef(2), vgf.ConstantRef(3)]

    segment = encoder.AddSegmentInfo(module, "test_segment", constants=constants)

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()

    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer)
    assert headerDecoder.IsValid()
    assert headerDecoder.CheckVersion()

    seqTableDecoder = vgf.CreateModelSequenceTableDecoder(
        buffer[
            headerDecoder.GetModuleTableOffset() + headerDecoder.GetModuleTableSize() :
        ]
    )

    assert seqTableDecoder.modelSequenceTableSize() == 1

    constantIndexes = seqTableDecoder.getSegmentConstantIndexes(segment.reference)
    assert len(constantIndexes) == 3
    assert constantIndexes[0] == constants[0].reference
    assert constantIndexes[1] == constants[1].reference
    assert constantIndexes[2] == constants[2].reference


def test_encode_decode_model_sequence_table_segment_dispatch_shape():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    module = encoder.AddPlaceholderModule(
        vgf.ModuleType.Graph, "test_module", "entry_point"
    )

    dispatchShape = np.array([1, 2, 3], dtype=np.uint32)

    segment = encoder.AddSegmentInfo(
        module, "test_segment", dispatchShape=dispatchShape
    )

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()

    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer)
    assert headerDecoder.IsValid()
    assert headerDecoder.CheckVersion()

    seqTableDecoder = vgf.CreateModelSequenceTableDecoder(
        buffer[
            headerDecoder.GetModuleTableOffset() + headerDecoder.GetModuleTableSize() :
        ]
    )

    assert seqTableDecoder.modelSequenceTableSize() == 1

    checkShape = seqTableDecoder.getSegmentDispatchShape(segment.reference)

    assert len(checkShape) == 3
    assert checkShape[0] == dispatchShape[0]
    assert checkShape[1] == dispatchShape[1]
    assert checkShape[2] == dispatchShape[2]


def test_encode_decode_model_sequence_table_segment_push_constant_range():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    module = encoder.AddPlaceholderModule(
        vgf.ModuleType.Graph, "test_module", "entry_point"
    )

    pushConstRange = encoder.AddPushConstRange(10, 0, 5)
    pushConstRanges = [pushConstRange]

    segment = encoder.AddSegmentInfo(
        module, "test_segment", pushConstRanges=pushConstRanges
    )

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()

    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer)
    assert headerDecoder.IsValid()
    assert headerDecoder.CheckVersion()

    seqTableDecoder = vgf.CreateModelSequenceTableDecoder(
        buffer[
            headerDecoder.GetModuleTableOffset() + headerDecoder.GetModuleTableSize() :
        ]
    )

    assert seqTableDecoder.modelSequenceTableSize() == 1

    pushConstantsRangeHandle = seqTableDecoder.getSegmentPushConstRange(
        segment.reference
    )

    assert seqTableDecoder.getPushConstRangesSize(pushConstantsRangeHandle) == 1
    assert (
        seqTableDecoder.getPushConstRangeStageFlags(
            pushConstantsRangeHandle, pushConstRange.reference
        )
        == 10
    )
    assert (
        seqTableDecoder.getPushConstRangeOffset(
            pushConstantsRangeHandle, pushConstRange.reference
        )
        == 0
    )
    assert (
        seqTableDecoder.getPushConstRangeSize(
            pushConstantsRangeHandle, pushConstRange.reference
        )
        == 5
    )
