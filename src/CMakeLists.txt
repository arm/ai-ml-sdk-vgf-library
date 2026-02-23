#
# SPDX-FileCopyrightText: Copyright 2023-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

set(VGF_SOURCES
    decoder_c_api.cpp
    decoder.cpp
    encoder.cpp
    logging_c_api.cpp
    logging.cpp
)

function(vgf_configure_target target_name)
    target_link_libraries(${target_name} PRIVATE $<BUILD_INTERFACE:vgf_schema>)
    target_include_directories(${target_name} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include-c>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    target_compile_options(${target_name} PRIVATE ${ML_SDK_VGF_LIB_COMPILE_OPTIONS})
endfunction()

add_library(vgf STATIC ${VGF_SOURCES})
vgf_configure_target(vgf)

if(ML_SDK_VGF_LIB_BUILD_SHARED)
    add_library(vgf_shared SHARED ${VGF_SOURCES})
    vgf_configure_target(vgf_shared)
    set_target_properties(vgf_shared PROPERTIES OUTPUT_NAME vgf)
endif()

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
