#
# SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

include(argparse)
include(version)

add_library(static_vgf_updater STATIC
    src/vgf_updater.cpp
)

target_include_directories(static_vgf_updater PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../utils/src>
)
target_link_libraries(static_vgf_updater PRIVATE vgf-utils vgf)
target_compile_options(static_vgf_updater PRIVATE ${ML_SDK_VGF_LIB_COMPILE_OPTIONS})

add_executable(vgf_updater
    src/main.cpp
)

target_link_libraries(vgf_updater PRIVATE static_vgf_updater argparse::argparse vgf-utils)
target_compile_options(vgf_updater PRIVATE ${ML_SDK_VGF_LIB_COMPILE_OPTIONS})

mlsdk_generate_version_header(
    TARGET vgf_updater
    SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/version.hpp.in"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/generated/version.hpp"
    DEPENDENCIES argparse
)

add_executable(${VGF_NAMESPACE}::vgf_updater ALIAS vgf_updater)

install(TARGETS vgf_updater EXPORT ${VGF_PACKAGE_NAME}Config)

if(ML_SDK_VGF_LIB_BUILD_TESTS)
    add_subdirectory(test)
endif()
