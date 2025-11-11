/*
 * SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "sample_utils.hpp"
#include "sample_vulkan.hpp"

#include <vgf/decoder.hpp>
#include <vgf/encoder.hpp>
#include <vgf/vulkan_helpers.generated.hpp>

#include <fstream>
#include <memory>
#include <string>

namespace mlsdk::vgflib::samples {

// Tutorial 1 - encode single maxpool graph example with tensors
std::string T1_encode_simple_graph_sample();

// Tutorial 2 - decode single maxpool graph example with tensors
void T2_decode_simple_graph_sample(const std::string &vgfFilename);

// Tutorial 3 - encode conv2d-rescale graph example with constant weights
std::string T3_encode_simple_graph_with_constants_sample();

// Tutorial 4 - decode conv2d-rescale graph example with constant weights
void T4_decode_simple_graph_with_constants_sample(const std::string &vgfFilename);

// Tutorial - decode single conv2d graph example with mmapped constants
// Tutorial - encode single compute shader example with images
// Tutorial - decode single compute shader example with images
// Tutorial - encode two compute segment example
// Tutorial - decode two compute segment example
// Tutorial - encode mixed compute/graph workload
// Tutorial - decode mixed compute/graph workload
// Tutorial - encode compute shader with push constants
// Tutorial - decode compute shader with push constants
// Tutorial - encode compute shader with specialisation constants
// Tutorial - decode compute shader with specialisation constants
// Tutorial - reconstructing depndencies

} // namespace mlsdk::vgflib::samples
