#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: Copyright 2024 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
""" Tests for VGF Constant. """
import io

import numpy as np
import pytest
import vgfpy as vgf

pretendVulkanHeaderVersion = 123

pytestmark = pytest.mark.constant_test


def test_encode_decode_constant():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    resourceRef = vgf.ResourceRef(42)
    constant = np.array(["a", "b"])
    sparsityDimension = 1

    constantRef = encoder.AddConstant(resourceRef, constant, sparsityDimension)

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()

    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer)
    assert headerDecoder.IsValid()
    assert headerDecoder.CheckVersion()

    constantDecoder = vgf.CreateConstantDecoder(
        buffer[headerDecoder.GetConstantsOffset() :]
    )

    assert constantDecoder.size() == 1
    assert (
        constantDecoder.getConstant(constantRef.reference).tobytes()
        == memoryview(constant).tobytes()
    )
    assert (
        constantDecoder.getConstantMrtIndex(constantRef.reference)
        == resourceRef.reference
    )
    assert constantDecoder.isSparseConstant(constantRef.reference)
    assert (
        constantDecoder.getConstantSparsityDimension(constantRef.reference)
        == sparsityDimension
    )


def test_encode_decode_non_Sparse_constant():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    resourceRef = vgf.ResourceRef(42)
    constant = np.array([1], dtype=np.uint8)

    constantRef = encoder.AddConstant(resourceRef, constant)

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()

    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer)
    assert headerDecoder.IsValid()
    assert headerDecoder.CheckVersion()

    constantDecoder = vgf.CreateConstantDecoder(
        buffer[headerDecoder.GetConstantsOffset() :]
    )

    assert constantDecoder.size() == 1
    assert constantDecoder.getConstant(constantRef.reference) == memoryview(constant)
    assert (
        constantDecoder.getConstantMrtIndex(constantRef.reference)
        == resourceRef.reference
    )
    assert not constantDecoder.isSparseConstant(constantRef.reference)


def test_encode_decode_empty_constant_section():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()

    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer)
    assert headerDecoder.IsValid()
    assert headerDecoder.CheckVersion()

    constantDecoder = vgf.CreateConstantDecoder(
        buffer[headerDecoder.GetConstantsOffset() :]
    )

    assert constantDecoder.size() == 0
