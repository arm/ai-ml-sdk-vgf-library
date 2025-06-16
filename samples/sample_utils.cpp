/*
 * SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sample_utils.hpp"

#if defined(HAS_SPIRV_HEADERS)
#    include "spirv-tools/libspirv.hpp"
#endif

namespace mlsdk::sampleutils {

std::vector<uint32_t> spirv_assemble([[maybe_unused]] const std::string &code) {
    std::vector<uint32_t> spirv;
#if defined(HAS_SPIRV_HEADERS)
    spvtools::SpirvTools core(SPV_ENV_UNIVERSAL_1_3);
    core.SetMessageConsumer([](spv_message_level_t, const char *, const spv_position_t &, const char *m) {
        std::cerr << "error: " << m << std::endl;
    });
    core.Assemble(code, &spirv);
    assert(core.Validate(spirv));
#else
    spirv = {0};
#endif

    return spirv;
}

} // namespace mlsdk::sampleutils
