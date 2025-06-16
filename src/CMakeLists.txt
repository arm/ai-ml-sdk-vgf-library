#
# SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

add_library(vgf STATIC
    decoder_c_api.cpp
    decoder.cpp
    encoder.cpp
    logging_c_api.cpp
    logging.cpp
)

target_link_libraries(vgf PRIVATE $<BUILD_INTERFACE:vgf_schema>)
target_include_directories(vgf PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include-c>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_options(vgf PRIVATE ${ML_SDK_VGF_LIB_COMPILE_OPTIONS})

if(ML_SDK_VGF_LIB_BUILD_PYLIB)
    message(STATUS "Python library build enabled")
    include(pybind11)
    pybind11_add_module(vgfpy
        vgf_py.cpp
        types_py.cpp
        encoder_py.cpp
        decoder_py.cpp)
    target_compile_options(vgfpy PRIVATE ${ML_SDK_VGF_LIB_COMPILE_OPTIONS})
    target_link_libraries(vgfpy PRIVATE
        vgf
        vgf_schema
        pybind11::module)
endif()
unset(ML_SDK_VGF_LIB_BUILD_PYLIB CACHE)

add_library(${VGF_NAMESPACE}::vgf ALIAS vgf)
install(TARGETS vgf EXPORT ${VGF_PACKAGE_NAME}Config)

install(DIRECTORY
    ../include/
    ../include-c/
    TYPE INCLUDE)
