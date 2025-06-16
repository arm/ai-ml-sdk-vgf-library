#
# SPDX-FileCopyrightText: Copyright 2024 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

include(version)

set(PYBIND11_PATH "PYBIND11-NOTFOUND" CACHE PATH "Path to Pybind11")
set(pybind11_VERSION "unknown")

if(EXISTS ${PYBIND11_PATH}/CMakeLists.txt)
    if(NOT TARGET pybind11)
        add_subdirectory(${PYBIND11_PATH} pybind11 EXCLUDE_FROM_ALL)
    endif()

    mlsdk_get_git_revision(${PYBIND11_PATH} pybind11_VERSION)
else()
    find_package(pybind11 REQUIRED CONFIG)
endif()
