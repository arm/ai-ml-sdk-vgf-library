#
# SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_HOST_SYSTEM_PROCESSOR})
set(CMAKE_CROSSCOMPILING OFF)
set(CMAKE_SYSTEM_NAME "Linux")

find_program(CMAKE_CXX_COMPILER clang++)
find_program(CMAKE_C_COMPILER clang)
find_program(CMAKE_LINKER lld)

include(${CMAKE_CURRENT_LIST_DIR}/gnu_compiler_options.cmake)
