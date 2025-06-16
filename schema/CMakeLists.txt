#
# SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/vgf_generated.h
    COMMAND ${FLATC_PATH}
        --cpp
        -I ${CMAKE_CURRENT_BINARY_DIR}
        -o ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/vgf.fbs
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/vgf.fbs
            ${FLATC_PATH}
    VERBATIM
)
add_library(vgf_schema INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/vgf_generated.h)
target_link_libraries(vgf_schema INTERFACE flatbuffers::flatbuffers)
target_include_directories(vgf_schema SYSTEM INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
