/*
 * SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include "vgf/version.h"

// A generated 0.0.0 version denotes a development build configured without
// ML_SDK_PACKAGE_VERSION. Any supplied component version must have had its
// public API fuzzer coverage reviewed explicitly.
static_assert((MLSDK_VGF_LIBRARY_API_VERSION_MAJOR == 0 && MLSDK_VGF_LIBRARY_API_VERSION_MINOR == 0 &&
               MLSDK_VGF_LIBRARY_API_VERSION_PATCH == 0) ||
                  (MLSDK_VGF_LIBRARY_API_VERSION_MAJOR == 0 && MLSDK_VGF_LIBRARY_API_VERSION_MINOR == 10 &&
                   MLSDK_VGF_LIBRARY_API_VERSION_PATCH == 0),
              "Public API version changed: update fuzzer coverage alongside the new API version.");

void FuzzCDecoders(const uint8_t *data, size_t size);
void FuzzCDecoderAccessors(const uint8_t *data, size_t size);
void FuzzCEncoderSmoke(const uint8_t *data, size_t size);

void FuzzCppDecoders(const uint8_t *data, size_t size);
void FuzzCppDecoderAccessors(const uint8_t *data, size_t size);
void FuzzCppEncoderSmoke(const uint8_t *data, size_t size);
