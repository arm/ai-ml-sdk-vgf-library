#
# SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

add_executable(vgf_samples
    samples.cpp
    sample_utils.cpp
    1_encode_simple_graph_sample.cpp
    2_decode_simple_graph_sample.cpp
    3_encode_simple_graph_with_constants_sample.cpp
    4_decode_simple_graph_with_constants_sample.cpp
)

target_link_libraries(vgf_samples PRIVATE vgf)
target_compile_options(vgf_samples PRIVATE ${ML_SDK_VGF_LIB_COMPILE_OPTIONS})
