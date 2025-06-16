#
# SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

set(GTEST_PATH "GTEST-NOTFOUND" CACHE PATH "Path to Google Test")

if(EXISTS ${GTEST_PATH}/CMakeLists.txt)
    if(NOT TARGET gtest)
        add_subdirectory(${GTEST_PATH} googletest SYSTEM EXCLUDE_FROM_ALL)
    endif()
else()
    find_package(GTest)

    if(NOT GTest_FOUND)
        message(WARNING "Could not find GTest library")
    endif()
endif()

include(GoogleTest)
