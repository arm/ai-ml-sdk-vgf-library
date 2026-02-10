/*
 * SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "fuzzers.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (data == nullptr || size == 0) {
        return 0;
    }

    // C++ surface
    FuzzCppDecoders(data, size);
    FuzzCppDecoderAccessors(data, size);

    // C surface
    FuzzCDecoders(data, size);
    FuzzCDecoderAccessors(data, size);

    return 0;
}
