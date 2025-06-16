#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: Copyright 2024 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
""" Tests for VGF Module Table. """
import io

import numpy as np
import pytest
import vgfpy as vgf

pretendVulkanHeaderVersion = 123

pytestmark = pytest.mark.module_table_test


def test_encode_decode_module_table_empty():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()

    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer)
    assert headerDecoder.IsValid()
    assert headerDecoder.CheckVersion()

    moduleDecoder = vgf.CreateModuleTableDecoder(
        buffer[headerDecoder.GetModuleTableOffset() :]
    )

    assert moduleDecoder.size() == 0


def test_encode_decode_module_table_single():

    code = np.array([1, 2, 3, 4], dtype=np.uint32)

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    module = encoder.AddModule(vgf.ModuleType.Graph, "test", "main", code)

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()

    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer)
    assert headerDecoder.IsValid()
    assert headerDecoder.CheckVersion()

    moduleIndex = module.reference

    moduleDecoder = vgf.CreateModuleTableDecoder(
        buffer[headerDecoder.GetModuleTableOffset() :]
    )

    numModules = moduleDecoder.size()

    assert moduleIndex < numModules

    assert numModules == 1
    assert moduleDecoder.getModuleType(moduleIndex) == vgf.ModuleType.Graph
    assert moduleDecoder.getModuleName(moduleIndex) == "test"
    assert moduleDecoder.hasSPIRV(moduleIndex)
    assert moduleDecoder.getModuleEntryPoint(moduleIndex) == "main"
    assert moduleDecoder.getModuleCode(moduleIndex) == memoryview(code)


def test_encode_decode_module_table_single_placeholder():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    module = encoder.AddPlaceholderModule(vgf.ModuleType.Compute, "test", "main")

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()

    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer)
    assert headerDecoder.IsValid()
    assert headerDecoder.CheckVersion()

    moduleIndex = module.reference

    moduleDecoder = vgf.CreateModuleTableDecoder(
        buffer[headerDecoder.GetModuleTableOffset() :]
    )

    assert moduleDecoder.size() == 1
    assert moduleDecoder.getModuleType(moduleIndex) == vgf.ModuleType.Compute
    assert moduleDecoder.getModuleName(moduleIndex) == "test"
    assert moduleDecoder.hasSPIRV(moduleIndex) == False
    assert moduleDecoder.getModuleEntryPoint(moduleIndex) == "main"
    assert moduleDecoder.getModuleCode(moduleIndex) is None
