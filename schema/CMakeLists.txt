#
# SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

add_library(vgf_schema INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vgf_generated.h)
target_link_libraries(vgf_schema INTERFACE flatbuffers::flatbuffers)
target_include_directories(vgf_schema SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
