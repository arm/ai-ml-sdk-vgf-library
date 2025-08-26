#
# SPDX-FileCopyrightText: Copyright 2022-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

include(argparse)
include(json)
include(version)

add_library(static_vgf_dump STATIC
    src/vgf_dump.cpp
)

target_include_directories(static_vgf_dump PUBLIC
    include
)
target_link_libraries(static_vgf_dump PUBLIC vgf nlohmann_json::nlohmann_json)
target_compile_options(static_vgf_dump PRIVATE ${ML_SDK_VGF_LIB_COMPILE_OPTIONS})

add_executable(vgf_dump
    src/main.cpp
)

target_link_libraries(vgf_dump PRIVATE static_vgf_dump argparse::argparse vgf-utils)
target_compile_options(vgf_dump PRIVATE ${ML_SDK_VGF_LIB_COMPILE_OPTIONS})

mlsdk_generate_version_header(
    TARGET vgf_dump
    SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/version.hpp.in"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/generated/version.hpp"
    DEPENDENCIES
        argparse
        nlohmann_json
)

add_executable(${VGF_NAMESPACE}::vgf_dump ALIAS vgf_dump)

install(TARGETS vgf_dump EXPORT ${VGF_PACKAGE_NAME}Config)
