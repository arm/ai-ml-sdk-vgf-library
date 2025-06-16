#
# SPDX-FileCopyrightText: Copyright 2024 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

include(version)

set(ARGPARSE_PATH "ARGPARSE-NOTFOUND" CACHE PATH "Path to Argparse")
set(argparse_VERSION "unknown")

if(EXISTS ${ARGPARSE_PATH}/CMakeLists.txt)
    if(NOT TARGET argparse)
        add_subdirectory(${ARGPARSE_PATH} argparse EXCLUDE_FROM_ALL)
    endif()

    mlsdk_get_git_revision(${ARGPARSE_PATH} argparse_VERSION)
else()
    find_package(argparse REQUIRED CONFIG)
endif()
