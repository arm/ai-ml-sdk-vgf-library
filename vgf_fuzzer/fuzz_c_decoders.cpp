/*
 * SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "fuzzers.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "vgf/decoder.h"

void FuzzCDecoders(const uint8_t *data, size_t size) {
    std::vector<uint8_t> hdrMem(mlsdk_decoder_header_decoder_mem_reqs());
    (void)mlsdk_decoder_create_header_decoder(data, static_cast<uint64_t>(mlsdk_decoder_header_size()), size,
                                              hdrMem.data());

    std::vector<uint8_t> modMem(mlsdk_decoder_module_table_decoder_mem_reqs());
    (void)mlsdk_decoder_create_module_table_decoder(data, size, modMem.data());

    std::vector<uint8_t> seqMem(mlsdk_decoder_model_sequence_decoder_mem_reqs());
    (void)mlsdk_decoder_create_model_sequence_decoder(data, size, seqMem.data());

    std::vector<uint8_t> mrtMem(mlsdk_decoder_model_resource_table_decoder_mem_reqs());
    (void)mlsdk_decoder_create_model_resource_table_decoder(data, size, mrtMem.data());

    std::vector<uint8_t> constMem(mlsdk_decoder_constant_table_decoder_mem_reqs());
    (void)mlsdk_decoder_create_constant_table_decoder(data, size, constMem.data());
}

void FuzzCDecoderAccessors(const uint8_t *data, size_t size) {
    std::vector<uint8_t> hdrMem(mlsdk_decoder_header_decoder_mem_reqs());
    auto *header = mlsdk_decoder_create_header_decoder(data, static_cast<uint64_t>(mlsdk_decoder_header_size()), size,
                                                       hdrMem.data());
    if (!header) {
        return;
    }

    mlsdk_decoder_is_header_valid(header);
    mlsdk_decoder_is_header_compatible(header);
    mlsdk_decoder_is_latest_version(header);

    mlsdk_decoder_vgf_version ver{};
    mlsdk_decoder_get_header_version(header, &ver);
    mlsdk_decoder_get_header_major(header);
    mlsdk_decoder_get_header_minor(header);
    mlsdk_decoder_get_header_patch(header);

    mlsdk_vk_header_version vkVer{};
    mlsdk_decoder_get_encoder_vk_header_version(header, &vkVer);

    /**************************************************************************/
    mlsdk_decoder_vgf_section_info mod{};
    mlsdk_decoder_get_header_section_info(header, mlsdk_decoder_section_modules, &mod);
    const uint64_t modOffset = static_cast<size_t>(mod.offset);
    const uint64_t modSize = static_cast<size_t>(mod.size);
    const size_t modMemSize = mlsdk_decoder_module_table_decoder_mem_reqs();
    std::vector<uint8_t> modMem(modMemSize);
    auto *modDec = mlsdk_decoder_create_module_table_decoder(data + modOffset, modSize, modMem.data());
    if (modDec) {
        const auto count = mlsdk_decoder_get_module_table_num_entries(modDec);
        if (count > 0) {
            const auto idx = static_cast<uint32_t>(std::min<size_t>(count - 1, UINT32_MAX));
            mlsdk_decoder_get_module_type(modDec, idx);
            mlsdk_decoder_get_module_name(modDec, idx);
            mlsdk_decoder_get_module_entry_point(modDec, idx);
            mlsdk_decoder_module_is_spirv(modDec, idx);
            mlsdk_decoder_module_has_spirv_code(modDec, idx);
            mlsdk_decoder_module_is_glsl(modDec, idx);
            mlsdk_decoder_module_has_glsl_code(modDec, idx);
            mlsdk_decoder_module_is_hlsl(modDec, idx);
            mlsdk_decoder_module_has_hlsl_code(modDec, idx);
            mlsdk_decoder_get_module_glsl_code(modDec, idx);
            mlsdk_decoder_get_module_hlsl_code(modDec, idx);
            mlsdk_decoder_spirv_code code{};
            mlsdk_decoder_get_spirv_module_code(modDec, idx, &code);
        }
    }

    /**************************************************************************/
    mlsdk_decoder_vgf_section_info mrt{};
    mlsdk_decoder_get_header_section_info(header, mlsdk_decoder_section_resources, &mrt);
    const uint64_t mrtOffset = static_cast<size_t>(mrt.offset);
    const uint64_t mrtSize = static_cast<size_t>(mrt.size);
    const size_t mrtMemSize = mlsdk_decoder_model_resource_table_decoder_mem_reqs();
    std::vector<uint8_t> mrtMem(mrtMemSize);
    auto *mrtDec = mlsdk_decoder_create_model_resource_table_decoder(data + mrtOffset, mrtSize, mrtMem.data());
    if (mrtDec) {
        const auto count = mlsdk_decoder_get_model_resource_table_num_entries(mrtDec);
        if (count > 0) {
            const auto idx = static_cast<uint32_t>(std::min<size_t>(count - 1, UINT32_MAX));
            mlsdk_decoder_get_vk_descriptor_type(mrtDec, idx);
            mlsdk_decoder_get_vk_format(mrtDec, idx);
            mlsdk_decoder_model_resource_table_get_category(mrtDec, idx);
            mlsdk_decoder_tensor_dimensions dims{};
            mlsdk_decoder_model_resource_table_get_tensor_shape(mrtDec, idx, &dims);
            mlsdk_decoder_model_resource_table_get_tensor_strides(mrtDec, idx, &dims);
        }
    }

    /**************************************************************************/
    mlsdk_decoder_vgf_section_info con{};
    mlsdk_decoder_get_header_section_info(header, mlsdk_decoder_section_constants, &con);
    const uint64_t constOffset = static_cast<size_t>(con.offset);
    const uint64_t constSize = static_cast<size_t>(con.size);
    const size_t constMemSize = mlsdk_decoder_constant_table_decoder_mem_reqs();
    std::vector<uint8_t> constMem(constMemSize);
    auto *constDec = mlsdk_decoder_create_constant_table_decoder(data + constOffset, constSize, constMem.data());
    if (constDec) {
        const auto count = mlsdk_decoder_get_constant_table_num_entries(constDec);
        if (count > 0) {
            const auto idx = static_cast<uint32_t>(std::min<size_t>(count - 1, UINT32_MAX));
            mlsdk_decoder_constant_data data{};
            mlsdk_decoder_constant_table_get_data(constDec, idx, &data);
            mlsdk_decoder_constant_table_get_mrt_index(constDec, idx);
            mlsdk_decoder_constant_table_is_sparse(constDec, idx);
            mlsdk_decoder_constant_table_get_sparsity_dimension(constDec, idx);
        }
    }

    /**************************************************************************/
    mlsdk_decoder_vgf_section_info seq{};
    mlsdk_decoder_get_header_section_info(header, mlsdk_decoder_section_model_sequence, &seq);
    const uint64_t seqOffset = static_cast<size_t>(seq.offset);
    const uint64_t seqSize = static_cast<size_t>(seq.size);
    const size_t seqMemSize = mlsdk_decoder_model_sequence_decoder_mem_reqs();
    std::vector<uint8_t> seqMem(seqMemSize);
    auto *seqDec = mlsdk_decoder_create_model_sequence_decoder(data + seqOffset, seqSize, seqMem.data());
    if (seqDec) {
        const auto count = mlsdk_decoder_get_model_sequence_table_size(seqDec);
        if (count > 0) {
            const auto idx = static_cast<uint32_t>(std::min<size_t>(count - 1, UINT32_MAX));
            const auto descCount = mlsdk_decoder_model_sequence_get_segment_descriptorset_info_size(seqDec, idx);
            if (descCount > 0) {
                mlsdk_decoder_model_sequence_get_segment_descriptorset_index(seqDec, idx, 0);
                const auto dIdx = static_cast<uint32_t>(std::min<size_t>(descCount - 1, UINT32_MAX));
                mlsdk_decoder_model_sequence_get_segment_descriptorset_index(seqDec, idx, dIdx);
            }
            mlsdk_decoder_constant_indexes constIdx{};
            mlsdk_decoder_model_sequence_get_segment_constant_indexes(seqDec, idx, &constIdx);
            mlsdk_decoder_model_sequence_get_segment_type(seqDec, idx);
            mlsdk_decoder_model_sequence_get_segment_name(seqDec, idx);
            mlsdk_decoder_model_sequence_get_segment_module_index(seqDec, idx);
            mlsdk_decoder_dispatch_shape shape{};
            mlsdk_decoder_model_sequence_get_segment_dispatch_shape(seqDec, idx, &shape);

            auto descSlotsHandle = mlsdk_decoder_model_sequence_get_segment_descriptor_binding_slot(seqDec, idx, 0);
            if (descSlotsHandle) {
                const auto bindCount = mlsdk_decoder_binding_slot_size(seqDec, descSlotsHandle);
                if (bindCount > 0) {
                    const auto bIdx = static_cast<uint32_t>(std::min<size_t>(bindCount - 1, UINT32_MAX));
                    mlsdk_decoder_binding_slot_binding_id(seqDec, descSlotsHandle, bIdx);
                    mlsdk_decoder_binding_slot_mrt_index(seqDec, descSlotsHandle, bIdx);
                }
            }

            auto segInputsHandle = mlsdk_decoder_model_sequence_get_segment_input_binding_slot(seqDec, idx);
            if (segInputsHandle) {
                mlsdk_decoder_binding_slot_size(seqDec, segInputsHandle);
            }
            auto segOutputsHandle = mlsdk_decoder_model_sequence_get_segment_output_binding_slot(seqDec, idx);
            if (segOutputsHandle) {
                mlsdk_decoder_binding_slot_size(seqDec, segOutputsHandle);
            }

            auto modelInputsHandle = mlsdk_decoder_model_sequence_get_input_binding_slot(seqDec);
            if (modelInputsHandle) {
                mlsdk_decoder_binding_slot_size(seqDec, modelInputsHandle);
            }
            auto modelOutputsHandle = mlsdk_decoder_model_sequence_get_output_binding_slot(seqDec);
            if (modelOutputsHandle) {
                mlsdk_decoder_binding_slot_size(seqDec, modelOutputsHandle);
            }

            auto nameInputs = mlsdk_decoder_model_sequence_get_input_names(seqDec);
            if (nameInputs) {
                auto namesInputsSize = mlsdk_decoder_model_sequence_get_names_size(seqDec, nameInputs);
                if (namesInputsSize > 0) {
                    mlsdk_decoder_model_sequence_get_name(seqDec, nameInputs, 0);
                }
            }

            auto nameOutputs = mlsdk_decoder_model_sequence_get_output_names(seqDec);
            if (nameOutputs) {
                auto namesOutputsSize = mlsdk_decoder_model_sequence_get_names_size(seqDec, nameOutputs);
                if (namesOutputsSize > 0) {
                    mlsdk_decoder_model_sequence_get_name(seqDec, nameOutputs, 0);
                }
            }

            auto pcrhHandle = mlsdk_decoder_model_sequence_get_segment_push_constant_range(seqDec, idx);
            if (pcrhHandle) {
                const auto pcrCount = mlsdk_decoder_get_push_constant_ranges_size(seqDec, pcrhHandle);
                if (pcrCount > 0) {
                    const auto pIdx = static_cast<uint32_t>(std::min<size_t>(pcrCount - 1, UINT32_MAX));
                    mlsdk_decoder_get_push_constant_range_stage_flags(seqDec, pcrhHandle, pIdx);
                    mlsdk_decoder_get_push_constant_range_offset(seqDec, pcrhHandle, pIdx);
                    mlsdk_decoder_get_push_constant_range_size(seqDec, pcrhHandle, pIdx);
                }
            }
        }
    }
}
