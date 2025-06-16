#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: Copyright 2024 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
""" Tests for VGF Model Resource. """
import io

import numpy as np
import pytest
import vgfpy as vgf

pretendVulkanHeaderVersion = 123

pytestmark = pytest.mark.model_resource_test


def test_encode_decode_model_resource_table_empty():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()

    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer)
    assert headerDecoder.IsValid()
    assert headerDecoder.CheckVersion()

    resTableDecoder = vgf.CreateModelResourceTableDecoder(
        buffer[headerDecoder.GetModelResourceTableOffset() :]
    )

    assert resTableDecoder.size() == 0


def test_encode_decode_model_resource_table():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    shape0 = np.array([0, 1, 2, 3], dtype=np.int64)
    strides0 = np.array([4, 5, 6, 7], dtype=np.int64)
    shape1 = np.array([8, 9, 10, 11], dtype=np.int64)
    strides1 = np.array([12, 13, 14, 15], dtype=np.int64)

    VK_DESCRIPTOR_TYPE_STORAGE_IMAGE = 3
    VK_FORMAT_R4G4_UNORM_PACK8 = 1
    VK_FORMAT_R4G4B4A4_UNORM_PACK16 = 12

    resource0 = encoder.AddInputResource(
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R4G4_UNORM_PACK8, shape0, strides0
    )
    resource1 = encoder.AddConstantResource(
        VK_FORMAT_R4G4B4A4_UNORM_PACK16, shape1, strides1
    )

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()

    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer)
    assert headerDecoder.IsValid()
    assert headerDecoder.CheckVersion()

    mrtIndex0 = resource0.reference

    mrtDecoder = vgf.CreateModelResourceTableDecoder(
        buffer[headerDecoder.GetModelResourceTableOffset() :]
    )

    assert mrtDecoder.size() == 2

    mrtIndex0 = resource0.reference
    mrtIndex1 = resource1.reference

    assert mrtDecoder.getCategory(mrtIndex0) == vgf.ResourceCategory.Input
    assert mrtDecoder.getTensorShape(mrtIndex0) == memoryview(shape0)
    assert mrtDecoder.getDescriptorType(mrtIndex0) == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
    assert mrtDecoder.getVkFormat(mrtIndex0) == VK_FORMAT_R4G4_UNORM_PACK8
    assert mrtDecoder.getTensorStride(mrtIndex0) == memoryview(strides0)

    assert mrtDecoder.getCategory(mrtIndex1) == vgf.ResourceCategory.Constant
    assert mrtDecoder.getTensorShape(resource1.reference) == memoryview(shape1)
    assert mrtDecoder.getDescriptorType(resource1.reference) is None
    assert (
        mrtDecoder.getVkFormat(resource1.reference) == VK_FORMAT_R4G4B4A4_UNORM_PACK16
    )
    assert mrtDecoder.getTensorStride(resource1.reference) == memoryview(strides1)


def test_encode_decode_model_resource_table_with_unknown_dimensions():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    shape0 = np.array([-1, -1, -1, -1], dtype=np.int64)
    shape1 = np.array([3, -1, 1, -1], dtype=np.int64)

    VK_DESCRIPTOR_TYPE_STORAGE_IMAGE = 3
    VK_FORMAT_R4G4_UNORM_PACK8 = 1
    VK_FORMAT_R4G4B4A4_UNORM_PACK16 = 12

    resource0 = encoder.AddInputResource(
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R4G4_UNORM_PACK8, shape0, []
    )
    resource1 = encoder.AddConstantResource(VK_FORMAT_R4G4B4A4_UNORM_PACK16, shape1, [])

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()

    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer)
    assert headerDecoder.IsValid()
    assert headerDecoder.CheckVersion()

    mrtIndex0 = resource0.reference

    mrtDecoder = vgf.CreateModelResourceTableDecoder(
        buffer[headerDecoder.GetModelResourceTableOffset() :]
    )

    assert mrtDecoder.size() == 2

    mrtIndex0 = resource0.reference
    mrtIndex1 = resource1.reference

    assert mrtDecoder.getCategory(mrtIndex0) == vgf.ResourceCategory.Input
    assert mrtDecoder.getTensorShape(mrtIndex0) == memoryview(shape0)
    assert mrtDecoder.getDescriptorType(mrtIndex0) == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
    assert mrtDecoder.getVkFormat(mrtIndex0) == VK_FORMAT_R4G4_UNORM_PACK8
    assert mrtDecoder.getTensorStride(mrtIndex0) is None

    assert mrtDecoder.getCategory(mrtIndex1) == vgf.ResourceCategory.Constant
    assert mrtDecoder.getTensorShape(resource1.reference) == memoryview(shape1)
    assert mrtDecoder.getDescriptorType(resource1.reference) is None
    assert (
        mrtDecoder.getVkFormat(resource1.reference) == VK_FORMAT_R4G4B4A4_UNORM_PACK16
    )
    assert mrtDecoder.getTensorStride(resource1.reference) is None
