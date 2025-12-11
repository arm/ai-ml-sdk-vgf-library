#
# SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

set(TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/test_data")

add_executable(VGFupdaterTests
    vgf_updater_tests.cpp
)
target_link_libraries(VGFupdaterTests PRIVATE
    GTest::gtest_main
    static_vgf_updater
    vgf-utils
    vgf
)

target_compile_definitions(VGFupdaterTests PRIVATE TEST_DATA_DIR="${TEST_DATA_DIR}")

target_include_directories(VGFupdaterTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)
target_compile_options(VGFupdaterTests PRIVATE ${ML_SDK_VGF_LIB_COMPILE_OPTIONS})

gtest_discover_tests(VGFupdaterTests
    PROPERTIES
        LABELS VGFupdaterTests
        # Avoid concurrent file access issues
        RUN_SERIAL TRUE)

enable_testing()
