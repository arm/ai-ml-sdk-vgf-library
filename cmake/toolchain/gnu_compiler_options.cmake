#
# SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

# Compilation warnings
set(ML_SDK_VGF_LIB_COMPILE_OPTIONS -Werror -Wall -Wextra -Wsign-conversion -Wconversion -Wpedantic)

if(ML_SDK_VGF_LIB_GCC_SANITIZERS)
    message(STATUS "GCC Sanitizers enabled")
    add_compile_options(
        -fsanitize=undefined,address
        -fno-sanitize=alignment
        -fno-sanitize-recover=all
    )
    add_link_options(
        -fsanitize=undefined,address
    )
    unset(ML_SDK_VGF_LIB_GCC_SANITIZERS CACHE)
endif()
