#
# SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

include(argparse)
include(version)

add_library(static_vgf_converter STATIC
    src/vgf_converter.cpp
)

target_include_directories(static_vgf_converter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../utils/src>
)
target_link_libraries(static_vgf_converter PRIVATE vgf-utils vgf)
target_compile_options(static_vgf_converter PRIVATE ${ML_SDK_VGF_LIB_COMPILE_OPTIONS})

add_executable(vgf_converter
    src/main.cpp
)

target_link_libraries(vgf_converter PRIVATE static_vgf_converter argparse::argparse vgf-utils)
target_compile_options(vgf_converter PRIVATE ${ML_SDK_VGF_LIB_COMPILE_OPTIONS})

mlsdk_generate_version_header(
    TARGET vgf_converter
    SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/version.hpp.in"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/generated/version.hpp"
    DEPENDENCIES argparse
)

add_executable(${VGF_NAMESPACE}::vgf_converter ALIAS vgf_converter)

install(TARGETS vgf_converter EXPORT ${VGF_PACKAGE_NAME}Config)

if(ML_SDK_VGF_LIB_BUILD_TESTS)
    add_subdirectory(test)
endif()
