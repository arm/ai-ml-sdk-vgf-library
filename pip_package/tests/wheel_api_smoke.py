#
# SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
"""Round-trip a minimal VGF using the installed Python binding.

Run with: python -I pip_package/tests/wheel_api_smoke.py
"""

import io
from array import array

import vgfpy as vgf


def check_empty_vgf():
    encoder = vgf.CreateEncoder(123)
    encoder.Finish()
    stream = io.BytesIO()
    assert encoder.WriteTo(stream)

    buffer = stream.getbuffer()
    assert buffer.nbytes >= vgf.HeaderSize()
    decoder = vgf.CreateHeaderDecoder(buffer, vgf.HeaderSize(), buffer.nbytes)
    assert decoder is not None
    assert decoder.GetEncoderVulkanHeadersVersion() == 123
    assert decoder.IsLatestVersion()
    version = decoder.GetVersion()
    assert isinstance(version, vgf.FormatVersion)
    assert version.major == vgf.HEADER_MAJOR_VERSION_VALUE
    assert version.minor == vgf.HEADER_MINOR_VERSION_VALUE
    assert version.patch == vgf.HEADER_PATCH_VERSION_VALUE


def check_module_and_constant():
    encoder = vgf.CreateEncoder(123)
    # The encoder stores these words; this check does not execute a shader.
    code = array("I", [1, 2, 3, 4])
    module = encoder.AddModule(vgf.ModuleType.Compute, "smoke", "main", code)
    payload = array("B", [0, 1, 127, 255])
    resource = vgf.ResourceRef(42)
    constant = encoder.AddConstant(resource, payload)
    encoder.Finish()
    stream = io.BytesIO()
    assert encoder.WriteTo(stream)
    buffer = stream.getbuffer()
    header = vgf.CreateHeaderDecoder(buffer, vgf.HeaderSize(), buffer.nbytes)
    assert header is not None

    modules = vgf.CreateModuleTableDecoder(
        buffer[header.GetModuleTableOffset() :], header.GetModuleTableSize()
    )
    assert modules is not None
    assert modules.size() == 1
    assert modules.getModuleType(module.reference) == vgf.ModuleType.Compute
    assert modules.getModuleName(module.reference) == "smoke"
    assert modules.getModuleEntryPoint(module.reference) == "main"
    assert modules.getSPIRVModuleCode(module.reference).tobytes() == code.tobytes()

    constants = vgf.CreateConstantDecoder(
        buffer[header.GetConstantsOffset() :], header.GetConstantsSize()
    )
    assert constants is not None
    assert constants.size() == 1
    assert constants.getConstant(constant.reference).tobytes() == payload.tobytes()
    assert constants.getConstantMrtIndex(constant.reference) == resource.reference
    assert not constants.isSparseConstant(constant.reference)


def check_invalid_header():
    buffer = memoryview(bytes(vgf.HeaderSize()))
    assert vgf.CreateHeaderDecoder(buffer, vgf.HeaderSize(), buffer.nbytes) is None


def main():
    check_empty_vgf()
    check_module_and_constant()
    check_invalid_header()


if __name__ == "__main__":
    main()
