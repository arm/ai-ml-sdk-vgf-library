#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
""" Tests for VGF Header. """
import io
import tempfile

import numpy as np
import pytest
import vgfpy as vgf

pretendVulkanHeaderVersion = 123

pytestmark = pytest.mark.header_test


def test_encode_decode_header():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()
    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer)
    assert headerDecoder.IsValid()
    assert headerDecoder.CheckVersion()

    assert headerDecoder.GetEncoderVulkanHeadersVersion() == pretendVulkanHeaderVersion

    assert headerDecoder.GetMajor() == vgf.HEADER_MAJOR_VERSION_VALUE
    assert headerDecoder.GetMinor() == vgf.HEADER_MINOR_VERSION_VALUE
    assert headerDecoder.GetPatch() == vgf.HEADER_PATCH_VERSION_VALUE
    assert headerDecoder.IsLatestVersion()

    assert headerDecoder.GetModuleTableSize() > 0
    assert headerDecoder.GetModuleTableOffset() == vgf.HEADER_HEADER_SIZE_VALUE

    assert headerDecoder.GetModelSequenceTableSize() > 0
    assert (
        headerDecoder.GetModelSequenceTableOffset()
        == vgf.HEADER_HEADER_SIZE_VALUE + headerDecoder.GetModuleTableSize()
    )

    assert headerDecoder.GetModelResourceTableSize() > 0
    assert (
        headerDecoder.GetModelResourceTableOffset()
        == headerDecoder.GetModelSequenceTableOffset()
        + headerDecoder.GetModelSequenceTableSize()
    )

    assert headerDecoder.GetConstantsSize() > 0
    assert (
        headerDecoder.GetConstantsOffset()
        == headerDecoder.GetModelResourceTableOffset()
        + headerDecoder.GetModelResourceTableSize()
    )


def test_decode_wrong_magic():

    wrongHeader = np.repeat(0, vgf.HEADER_HEADER_SIZE_VALUE)
    stream = io.BytesIO(wrongHeader)

    buffer = stream.getbuffer()

    decoder = vgf.CreateHeaderDecoder(buffer)
    assert not decoder.IsValid()
    assert not decoder.CheckVersion()

    vgf1 = vgf.FourCC("V", "G", "F", "1")
    assert vgf1.a == "V"
    assert vgf1.b == "G"
    assert vgf1.c == "F"
    assert vgf1.d == "1"

    assert vgf1 == vgf.FourCC("V", "G", "F", "1")


def test_encode_fails_to_write(tmp_path):

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    encoder.Finish()

    # Write should fail for non IO stream objects
    vgfBytes = bytes()
    with pytest.raises(RuntimeError):
        encoder.WriteTo(vgfBytes)

    # Write should fail when stream is not open for writing
    tmp_vgf = tmp_path / "test.vgf"
    tmp_vgf.touch()

    vgfStream = io.FileIO(tmp_vgf, mode="rb")

    with pytest.raises(io.UnsupportedOperation):
        encoder.WriteTo(vgfStream)
