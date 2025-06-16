#
# SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

include(CTest)

add_executable(VGFLibTests
  constant_tests.cpp
  header_tests.cpp
  logging_tests.cpp
  model_resource_tests.cpp
  model_sequence_tests.cpp
  module_table_tests.cpp
  section_index_table_tests.cpp
)
target_link_libraries(VGFLibTests PRIVATE
    GTest::gtest_main
    vgf
    vgf_schema
    $<$<PLATFORM_ID:Android>:log>)
target_include_directories(VGFLibTests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_compile_options(VGFLibTests PRIVATE ${ML_SDK_VGF_LIB_COMPILE_OPTIONS})

gtest_discover_tests(VGFLibTests PROPERTIES LABELS VGFLibTests)

enable_testing()
