#
# SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

set(TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/test_data")

add_executable(VGFConverterTests
    vgf_converter_tests.cpp
)
target_link_libraries(VGFConverterTests PRIVATE
    GTest::gtest_main
    static_vgf_converter
    vgf-utils
    vgf
)

target_compile_definitions(VGFConverterTests PRIVATE TEST_DATA_DIR="${TEST_DATA_DIR}")

target_include_directories(VGFConverterTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)
target_compile_options(VGFConverterTests PRIVATE ${ML_SDK_VGF_LIB_COMPILE_OPTIONS})

gtest_discover_tests(VGFConverterTests PROPERTIES LABELS VGFConverterTests)

enable_testing()
