#
# SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

if(PYBIND11_FINDPYTHON)
    find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
endif()
include(pybind11)
pybind11_add_module(vgfpy
    "${CMAKE_CURRENT_LIST_DIR}/../vgf_py.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../types_py.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../encoder_py.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../decoder_py.cpp")
target_compile_options(vgfpy PRIVATE ${ML_SDK_VGF_LIB_COMPILE_OPTIONS})
vgf_enable_coverage(vgfpy)
target_link_libraries(vgfpy PRIVATE
    vgf
    vgf_schema
    pybind11::module)
