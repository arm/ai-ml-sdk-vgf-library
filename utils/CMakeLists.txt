# SPDX-FileCopyrightText: Copyright 2025-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
###############################################################################
# VGF Utilities Library
###############################################################################
add_library(vgf-utils STATIC)
target_sources(vgf-utils PRIVATE
    src/memory_map.cpp
    src/numpy.cpp
    src/temp_folder.cpp
    src/parse_vgf.cpp)
target_include_directories(vgf-utils PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
add_library(${VGF_NAMESPACE}::vgf-utils ALIAS vgf-utils)

install(TARGETS vgf-utils EXPORT ${VGF_PACKAGE_NAME}Config)
