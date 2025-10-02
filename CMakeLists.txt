#
# SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
cmake_minimum_required(VERSION 3.22)
project(VGF
    DESCRIPTION "VGF Library"
    LANGUAGES CXX C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

set(VGF_PACKAGE_NAME ${PROJECT_NAME})
set(VGF_NAMESPACE ${PROJECT_NAME})
if(ML_SDK_GENERATE_CPACK)
    set(VGF_PACKAGE_NAME "ai_ml_sdk_for_vulkan")
else()
    set(CPACK_PACKAGE_NAME "ai_ml_sdk_vgf_library")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
if(DEFINED ANDROID_PLATFORM)
    message(WARNING "Cross-compiling for Android is currently an experimental feature.")
endif()
# Compile defines
add_compile_definitions(
    "$<$<NOT:$<CONFIG:DEBUG>>:NDEBUG>")

#########################################################################################

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(flatbuffers)

#########################################################################################
# Options
option(ML_SDK_VGF_LIB_BUILD_DOCS "Build Documentation" OFF)
option(ML_SDK_VGF_LIB_BUILD_PYLIB "Build VGF Python Lib" OFF)
option(ML_SDK_VGF_LIB_BUILD_TESTS "Build Format unit tests" OFF)
option(ML_SDK_VGF_LIB_BUILD_TOOLS "Build VGF tools" ON)
option(ML_SDK_VGF_LIB_ENABLE_LINT "Enable linting using cppcheck" OFF)

# exclude schema from linting, to avoid failures in flatbuffers
add_subdirectory(schema)
add_subdirectory(utils)

# Enable cppcheck
if(ML_SDK_VGF_LIB_ENABLE_LINT)
    message(STATUS "Cppcheck is enabled.")
    find_program(CPPCHECK_EXECUTABLE NAMES cppcheck)
    include(ProcessorCount)
    ProcessorCount(NCORES)
    set(CMAKE_CXX_CPPCHECK_OPTIONS
        -j${NCORES}
        --force
        --enable=style,performance,portability,information
        --quiet
        --inline-suppr
        --error-exitcode=1
        --suppress=unreadVariable
        --suppress=unmatchedSuppression
        --suppress=noValidConfiguration
        --suppress=preprocessorErrorDirective
        --suppress=*:${CMAKE_BINARY_DIR}*
        )

    if(EXISTS ${FLATBUFFERS_PATH})
        list(APPEND CMAKE_CXX_CPPCHECK_OPTIONS --suppress=*:*${FLATBUFFERS_PATH}*)
    endif()

    if(EXISTS ${ARGPARSE_PATH})
        list(APPEND CMAKE_CXX_CPPCHECK_OPTIONS --suppress=*:*${ARGPARSE_PATH}*)
    endif()

    if(EXISTS ${JSON_PATH})
        list(APPEND CMAKE_CXX_CPPCHECK_OPTIONS --suppress=*:*${JSON_PATH}*)
    endif()

    if(EXISTS ${GTEST_PATH})
        list(APPEND CMAKE_CXX_CPPCHECK_OPTIONS --suppress=*:*${GTEST_PATH}*)
    endif()

    if(ML_SDK_VGF_LIB_BUILD_PYLIB AND EXISTS ${PYBIND11_PATH})
        list(APPEND CMAKE_CXX_CPPCHECK_OPTIONS --suppress=*:*${PYBIND11_PATH}*)
    endif()

    set(CMAKE_C_CPPCHECK "${CPPCHECK_EXECUTABLE}" "${CMAKE_CXX_CPPCHECK_OPTIONS}" --std=c99)
    set(CMAKE_CXX_CPPCHECK "${CPPCHECK_EXECUTABLE}" "${CMAKE_CXX_CPPCHECK_OPTIONS}" --std=c++17)
endif()
unset(ML_SDK_VGF_LIB_ENABLE_LINT CACHE)

add_subdirectory(src)

add_subdirectory(samples)

if(ML_SDK_VGF_LIB_BUILD_TOOLS OR ML_SDK_BUILD_DOCS OR ML_SDK_VGF_LIB_BUILD_DOCS)
    add_subdirectory(vgf_dump)
endif()

if(ML_SDK_VGF_LIB_BUILD_TESTS)
    include(gtest)

    add_subdirectory(test)
endif()

if(ML_SDK_BUILD_DOCS OR ML_SDK_VGF_LIB_BUILD_DOCS)
    include(docs/docs.cmake)
endif()
unset(ML_SDK_VGF_LIB_BUILD_DOCS CACHE)

###############################################################################
# Packaging
###############################################################################
include(package)
if(NOT ML_SDK_GENERATE_CPACK)
    mlsdk_package(PACKAGE_NAME ${VGF_PACKAGE_NAME}
        NAMESPACE ${VGF_NAMESPACE}
        LICENSES "${CMAKE_CURRENT_LIST_DIR}/LICENSES/Apache-2.0.txt")
endif()
unset(ANDROID_PLATFORM CACHE)
