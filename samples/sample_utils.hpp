/*
 * SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace mlsdk::sampleutils {

std::vector<uint32_t> spirv_assemble(const std::string &code);

template <typename T> inline size_t sizeof_vector(const std::vector<T> &vec) { return vec.size() * sizeof(T); }

} // namespace mlsdk::sampleutils
