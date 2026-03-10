#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: Copyright 2024-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
import io

import numpy as np
import pytest
import vgfpy as vgf

"""Tests for VGF Module Table."""

pretendVulkanHeaderVersion = 123

pytestmark = pytest.mark.module_table_test


def test_encode_decode_module_table_empty():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()

    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer, vgf.HeaderSize(), buffer.nbytes)
    assert headerDecoder is not None

    moduleDecoder = vgf.CreateModuleTableDecoder(
        buffer[headerDecoder.GetModuleTableOffset() :],
        headerDecoder.GetModuleTableSize(),
    )
    assert moduleDecoder is not None

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

    headerDecoder = vgf.CreateHeaderDecoder(buffer, vgf.HeaderSize(), buffer.nbytes)
    assert headerDecoder is not None

    moduleIndex = module.reference

    moduleDecoder = vgf.CreateModuleTableDecoder(
        buffer[headerDecoder.GetModuleTableOffset() :],
        headerDecoder.GetModuleTableSize(),
    )
    assert moduleDecoder is not None

    numModules = moduleDecoder.size()

    assert moduleIndex < numModules

    assert numModules == 1
    assert moduleDecoder.getModuleType(moduleIndex) == vgf.ModuleType.Graph
    assert moduleDecoder.getModuleName(moduleIndex) == "test"
    assert moduleDecoder.isSPIRV(moduleIndex)
    assert moduleDecoder.hasSPIRVCode(moduleIndex)
    assert moduleDecoder.isGLSL(moduleIndex) is False
    assert moduleDecoder.hasGLSLCode(moduleIndex) is False
    assert moduleDecoder.isHLSL(moduleIndex) is False
    assert moduleDecoder.hasHLSLCode(moduleIndex) is False
    assert moduleDecoder.getGLSLModuleCode(moduleIndex) == ""
    assert moduleDecoder.getHLSLModuleCode(moduleIndex) == ""
    assert moduleDecoder.getModuleEntryPoint(moduleIndex) == "main"
    assert moduleDecoder.getSPIRVModuleCode(moduleIndex) == memoryview(code)


def test_encode_decode_module_table_has_spirv_deprecated():

    code = np.array([1, 2, 3, 4], dtype=np.uint32)

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)
    module = encoder.AddModule(vgf.ModuleType.Graph, "test", "main", code)

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()

    headerDecoder = vgf.CreateHeaderDecoder(buffer, vgf.HeaderSize(), buffer.nbytes)
    assert headerDecoder is not None

    moduleDecoder = vgf.CreateModuleTableDecoder(
        buffer[headerDecoder.GetModuleTableOffset() :],
        headerDecoder.GetModuleTableSize(),
    )
    assert moduleDecoder is not None

    with pytest.deprecated_call(
        match=r"ModuleTableDecoder\.hasSPIRV is deprecated; use isSPIRV\(\)"
    ):
        assert moduleDecoder.hasSPIRV(module.reference)


def test_encode_decode_module_table_single_placeholder():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)

    module = encoder.AddModule(vgf.ModuleType.Compute, "test", "main")

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()

    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer, vgf.HeaderSize(), buffer.nbytes)
    assert headerDecoder is not None

    moduleIndex = module.reference

    moduleDecoder = vgf.CreateModuleTableDecoder(
        buffer[headerDecoder.GetModuleTableOffset() :],
        headerDecoder.GetModuleTableSize(),
    )
    assert moduleDecoder is not None

    assert moduleDecoder.size() == 1
    assert moduleDecoder.getModuleType(moduleIndex) == vgf.ModuleType.Compute
    assert moduleDecoder.getModuleName(moduleIndex) == "test"
    assert moduleDecoder.isSPIRV(moduleIndex)
    assert moduleDecoder.hasSPIRVCode(moduleIndex) is False
    assert moduleDecoder.isGLSL(moduleIndex) is False
    assert moduleDecoder.hasGLSLCode(moduleIndex) is False
    assert moduleDecoder.isHLSL(moduleIndex) is False
    assert moduleDecoder.hasHLSLCode(moduleIndex) is False
    assert moduleDecoder.getGLSLModuleCode(moduleIndex) == ""
    assert moduleDecoder.getHLSLModuleCode(moduleIndex) == ""
    assert moduleDecoder.getModuleEntryPoint(moduleIndex) == "main"
    assert moduleDecoder.getSPIRVModuleCode(moduleIndex) is None


def test_encode_decode_module_table_single_glsl_module():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)
    module = encoder.AddModule(
        vgf.ModuleType.Compute,
        "glsl",
        "main",
        vgf.ShaderType.Glsl,
        "void main(){}",
    )

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()
    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer, vgf.HeaderSize(), buffer.nbytes)
    assert headerDecoder is not None

    moduleDecoder = vgf.CreateModuleTableDecoder(
        buffer[headerDecoder.GetModuleTableOffset() :],
        headerDecoder.GetModuleTableSize(),
    )
    assert moduleDecoder is not None
    assert moduleDecoder.size() == 1
    assert moduleDecoder.getModuleType(module.reference) == vgf.ModuleType.Compute
    assert moduleDecoder.getModuleName(module.reference) == "glsl"
    assert moduleDecoder.getModuleEntryPoint(module.reference) == "main"
    assert moduleDecoder.isSPIRV(module.reference) is False
    assert moduleDecoder.hasSPIRVCode(module.reference) is False
    assert moduleDecoder.isGLSL(module.reference)
    assert moduleDecoder.hasGLSLCode(module.reference)
    assert moduleDecoder.isHLSL(module.reference) is False
    assert moduleDecoder.hasHLSLCode(module.reference) is False
    assert moduleDecoder.getGLSLModuleCode(module.reference) == "void main(){}"
    assert moduleDecoder.getHLSLModuleCode(module.reference) == ""
    assert moduleDecoder.getSPIRVModuleCode(module.reference) is None


def test_encode_decode_module_table_single_hlsl_module():

    encoder = vgf.CreateEncoder(pretendVulkanHeaderVersion)
    module = encoder.AddModule(
        vgf.ModuleType.Compute,
        "hlsl",
        "main",
        vgf.ShaderType.Hlsl,
        "[numthreads(1,1,1)] void main(){}",
    )

    encoder.Finish()

    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()
    assert buffer.nbytes >= vgf.HeaderSize()

    headerDecoder = vgf.CreateHeaderDecoder(buffer, vgf.HeaderSize(), buffer.nbytes)
    assert headerDecoder is not None

    moduleDecoder = vgf.CreateModuleTableDecoder(
        buffer[headerDecoder.GetModuleTableOffset() :],
        headerDecoder.GetModuleTableSize(),
    )
    assert moduleDecoder is not None
    assert moduleDecoder.size() == 1
    assert moduleDecoder.getModuleType(module.reference) == vgf.ModuleType.Compute
    assert moduleDecoder.getModuleName(module.reference) == "hlsl"
    assert moduleDecoder.getModuleEntryPoint(module.reference) == "main"
    assert moduleDecoder.isSPIRV(module.reference) is False
    assert moduleDecoder.hasSPIRVCode(module.reference) is False
    assert moduleDecoder.isGLSL(module.reference) is False
    assert moduleDecoder.hasGLSLCode(module.reference) is False
    assert moduleDecoder.isHLSL(module.reference)
    assert moduleDecoder.hasHLSLCode(module.reference)
    assert moduleDecoder.getGLSLModuleCode(module.reference) == ""
    assert (
        moduleDecoder.getHLSLModuleCode(module.reference)
        == "[numthreads(1,1,1)] void main(){}"
    )
    assert moduleDecoder.getSPIRVModuleCode(module.reference) is None
