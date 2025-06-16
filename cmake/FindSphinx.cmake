#
# SPDX-FileCopyrightText: Copyright 2023-2024 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

find_program(
    SPHINX_EXECUTABLE
    NAMES sphinx-build sphinx-build2 sphinx-build.exe
    DOC "Path to the Sphinx's executable")

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)

find_package_handle_standard_args(
    Sphinx
    REQUIRED_VARS SPHINX_EXECUTABLE
    HANDLE_COMPONENTS)
