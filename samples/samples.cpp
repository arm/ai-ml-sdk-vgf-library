/*
 * SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "samples.hpp"

#include <iostream>

int main() {
    using namespace mlsdk::vgflib;
    // Tutorial 1
    auto tutorial1Vgf = samples::T1_encode_simple_graph_sample();

    // Tutorial 2
    samples::T2_decode_simple_graph_sample(tutorial1Vgf);

    // Tutorial 3
    auto tutorial3Vgf = samples::T3_encode_simple_graph_with_constants_sample();

    // Tutorial 4
    samples::T4_decode_simple_graph_with_constants_sample(tutorial3Vgf);

    std::cout << "Samples execution complete." << std::endl;

    return 0;
}
