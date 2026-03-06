#
# SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

if(NOT ML_SDK_VGF_LIB_ENABLE_FUZZER)
    return()
endif()

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "vgf_fuzzer requires clang/LLVM libFuzzer.")
endif()

message(STATUS "Building vgf_fuzzer (clang/libFuzzer + ASan + UBSan)")

add_executable(vgf_fuzzer
    main.cpp
    fuzz_c_decoders.cpp
    fuzz_cpp_decoders.cpp
)

target_link_libraries(vgf_fuzzer PRIVATE
    vgf
    vgf_schema
)

target_compile_options(vgf_fuzzer PRIVATE
    ${ML_SDK_VGF_LIB_COMPILE_OPTIONS}
    -fsanitize=fuzzer,address,undefined
    -fsanitize-recover=address,undefined
    -fno-omit-frame-pointer
    -fno-sanitize=leak
)
target_link_options(vgf_fuzzer PRIVATE
    -fsanitize=fuzzer,address,undefined
    -fsanitize-recover=address,undefined
    -fno-sanitize=leak
)
