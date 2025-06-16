#
# SPDX-FileCopyrightText: Copyright 2022-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_CROSSCOMPILING TRUE)

find_program(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
find_program(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
find_program(CMAKE_LINKER aarch64-linux-gnu-ld)

# Where to look for the target environment. More paths can be added here.
set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)
set(CMAKE_INCLUDE_PATH  /usr/include/aarch64-linux-gnu)
set(CMAKE_LIBRARY_PATH  /usr/lib/aarch64-linux-gnu)
set(CMAKE_PROGRAM_PATH  /usr/bin/aarch64-linux-gnu)

# Adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment only.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search headers and libraries in the target environment only.
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)

include(${CMAKE_CURRENT_LIST_DIR}/gnu_compiler_options.cmake)
