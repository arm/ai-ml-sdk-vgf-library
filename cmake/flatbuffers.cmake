#
# SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

include(version)

set(FLATBUFFERS_PATH "FLATBUFFERS-NOTFOUND" CACHE PATH "Path to Flatbuffers")
set(flatbuffers_VERSION "unknown")

if(EXISTS ${FLATBUFFERS_PATH}/CMakeLists.txt)
    if(NOT TARGET flatbuffers)
        option(FLATBUFFERS_BUILD_TESTS "" OFF)
        set(FLATBUFFERS_BUILD_FLATC ON CACHE BOOL "" FORCE)
        set(FLATBUFFERS_BUILD_FLATHASH OFF CACHE BOOL "" FORCE)

        add_subdirectory(${FLATBUFFERS_PATH} flatbuffers EXCLUDE_FROM_ALL)
        add_library(flatbuffers::flatbuffers ALIAS flatbuffers)
        add_executable(flatbuffers::flatc ALIAS flatc)
    endif()

    mlsdk_get_git_revision(${FLATBUFFERS_PATH} flatbuffers_VERSION)
else()
    find_package(flatbuffers REQUIRED CONFIG)

    if(NOT TARGET flatbuffers::flatbuffers AND TARGET flatbuffers)
        add_library(flatbuffers::flatbuffers ALIAS flatbuffers)
    endif()
endif()

if(TARGET flatbuffers::flatc AND NOT CMAKE_CROSSCOMPILING)
    set(FLATC_PATH $<TARGET_FILE:flatbuffers::flatc> CACHE INTERNAL "")
else()
    find_program(FLATC_PATH flatc23 NAMES flatc flatc23 PATH_SUFFIXES Release Debug REQUIRED)
endif()
