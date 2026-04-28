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
#include "vgf/encoder.h"

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
            mlsdk_vk_descriptor_type descriptorType = 0;
            (void)mlsdk_decoder_get_vk_descriptor_type(mrtDec, idx, &descriptorType);
            mlsdk_alias_group_id aliasGroupId = 0;
            (void)mlsdk_decoder_model_resource_table_get_alias_group_id(mrtDec, idx, &aliasGroupId);
            mlsdk_decoder_get_vk_format(mrtDec, idx);
            mlsdk_decoder_model_resource_table_get_category(mrtDec, idx);
            mlsdk_decoder_tensor_dimensions dims{};
            mlsdk_decoder_model_resource_table_get_tensor_shape(mrtDec, idx, &dims);
            mlsdk_decoder_model_resource_table_get_tensor_strides(mrtDec, idx, &dims);
            const auto samplerConfigHandle = mlsdk_decoder_model_resource_table_get_sampler_config_handle(mrtDec, idx);
            if (samplerConfigHandle != nullptr) {
                mlsdk_decoder_model_resource_table_sampler_config_get_min_filter(mrtDec, samplerConfigHandle);
                mlsdk_decoder_model_resource_table_sampler_config_get_mag_filter(mrtDec, samplerConfigHandle);
                mlsdk_decoder_model_resource_table_sampler_config_get_address_mode_u(mrtDec, samplerConfigHandle);
                mlsdk_decoder_model_resource_table_sampler_config_get_address_mode_v(mrtDec, samplerConfigHandle);
                mlsdk_decoder_model_resource_table_sampler_config_get_border_color(mrtDec, samplerConfigHandle);
            }
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

void FuzzCEncoderSmoke(const uint8_t *data, size_t size) {
    const auto vkHeaderVersion = size >= sizeof(uint16_t) ? static_cast<uint16_t>(data[0] | (data[1] << 8)) : 0;
    constexpr mlsdk_encoder_vk_descriptor_type combinedImageSamplerDescriptorType = 1;
    constexpr uint32_t samplerMinFilter = 0;
    constexpr uint32_t samplerMagFilter = 1;
    constexpr uint32_t samplerAddressModeU = 2;
    constexpr uint32_t samplerAddressModeV = 3;
    constexpr uint32_t samplerBorderColor = 4;
    constexpr mlsdk_encoder_alias_group_id aliasGroupId = 17;
    auto *encoder = mlsdk_encoder_create(vkHeaderVersion);
    if (encoder == nullptr) {
        return;
    }

    const uint32_t spirv[] = {0x07230203, 0x00010000};
    const auto spirvModule = mlsdk_encoder_add_spirv_module(encoder, mlsdk_encoder_module_type_compute, "spirv_module",
                                                            "main", spirv, std::size(spirv));
    (void)mlsdk_encoder_add_source_module(encoder, mlsdk_encoder_module_type_graph, "glsl_module", "main",
                                          mlsdk_encoder_shader_type_glsl, "void main(){}");
    (void)mlsdk_encoder_add_source_module(encoder, mlsdk_encoder_module_type_compute, "hlsl_module", "main",
                                          mlsdk_encoder_shader_type_hlsl, "[numthreads(1,1,1)] void main(){}");

    const int64_t shape[] = {1, 2};
    const int64_t strides[] = {2, 1};
    const auto inputResource =
        mlsdk_encoder_add_input_resource_with_alias_group(encoder, combinedImageSamplerDescriptorType, 0, shape,
                                                          std::size(shape), strides, std::size(strides), aliasGroupId);
    mlsdk_encoder_add_sampler_config(encoder, inputResource, samplerMinFilter, samplerMagFilter, samplerAddressModeU,
                                     samplerAddressModeV, samplerBorderColor);
    const auto outputResource = mlsdk_encoder_add_output_resource_with_alias_group(
        encoder, 0, 0, shape, std::size(shape), nullptr, 0, aliasGroupId);
    const auto intermediateResource = mlsdk_encoder_add_intermediate_resource_with_alias_group(
        encoder, 0, 0, shape, std::size(shape), nullptr, 0, aliasGroupId);
    const auto lateAssignedResource =
        mlsdk_encoder_add_intermediate_resource(encoder, 0, 0, shape, std::size(shape), nullptr, 0);
    mlsdk_encoder_set_alias_group(encoder, lateAssignedResource, aliasGroupId);
    const auto constantResource = mlsdk_encoder_add_constant_resource(encoder, 0, shape, std::size(shape), nullptr, 0);

    const uint8_t constantData[] = {1, 2, 3, 4};
    const auto constant = mlsdk_encoder_add_constant(encoder, constantResource, constantData, std::size(constantData),
                                                     MLSDK_ENCODER_CONSTANT_NOT_SPARSE_DIMENSION);

    const auto inputBinding = mlsdk_encoder_add_binding_slot(encoder, 0, inputResource);
    const auto outputBinding = mlsdk_encoder_add_binding_slot(encoder, 1, outputResource);
    const auto intermediateBinding = mlsdk_encoder_add_binding_slot(encoder, 2, intermediateResource);
    const auto constantBinding = mlsdk_encoder_add_binding_slot(encoder, 3, constantResource);

    const mlsdk_encoder_binding_slot_ref descriptorBindings[] = {inputBinding, outputBinding, intermediateBinding,
                                                                 constantBinding};
    const mlsdk_encoder_descriptor_set_info_ref descriptors[] = {
        mlsdk_encoder_add_descriptor_set_info(encoder, descriptorBindings, std::size(descriptorBindings), 0),
        mlsdk_encoder_add_descriptor_set_info(encoder, nullptr, 0, MLSDK_ENCODER_DESCRIPTOR_SET_INDEX_UNSPECIFIED)};

    const auto pushConstRange = mlsdk_encoder_add_push_const_range(encoder, 1, 0, 4);
    const uint32_t dispatchShape[] = {1, 1, 1};
    const mlsdk_encoder_binding_slot_ref inputs[] = {inputBinding};
    const mlsdk_encoder_binding_slot_ref outputs[] = {outputBinding};
    const mlsdk_encoder_constant_ref constants[] = {constant};
    const mlsdk_encoder_push_const_range_ref pushConstRanges[] = {pushConstRange};
    (void)mlsdk_encoder_add_segment_info(encoder, spirvModule, "segment", descriptors, std::size(descriptors), inputs,
                                         std::size(inputs), outputs, std::size(outputs), constants,
                                         std::size(constants), dispatchShape, pushConstRanges,
                                         std::size(pushConstRanges));

    const char *inputNames[] = {"input"};
    const char *outputNames[] = {"output"};
    mlsdk_encoder_add_model_sequence_inputs_outputs(encoder, inputs, std::size(inputs), inputNames,
                                                    std::size(inputNames), outputs, std::size(outputs), outputNames,
                                                    std::size(outputNames));
    mlsdk_encoder_finish(encoder);
    (void)mlsdk_encoder_write_to_file(encoder, "");
    mlsdk_encoder_destroy(encoder);
}
