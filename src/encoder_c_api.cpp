/*
 * SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf/encoder.h"
#include "vgf/encoder.hpp"
#include "vgf/types.hpp"

#include <array>
#include <cassert>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

using namespace mlsdk::vgflib;

struct mlsdk_encoder_s {
    explicit mlsdk_encoder_s(mlsdk_encoder_vk_header_version vkHeaderVersion)
        : encoder(CreateEncoder(vkHeaderVersion)) {}

    std::unique_ptr<Encoder> encoder;
};

namespace {

ModuleType convert_module_type(mlsdk_encoder_module_type type) {
    switch (type) {
    case mlsdk_encoder_module_type_compute:
        return ModuleType::COMPUTE;
    case mlsdk_encoder_module_type_graph:
        return ModuleType::GRAPH;
    default:
        assert(false && "unknown module type");
        return ModuleType::COMPUTE;
    }
}

ShaderType convert_shader_type(mlsdk_encoder_shader_type type) {
    switch (type) {
    case mlsdk_encoder_shader_type_glsl:
        return ShaderType::GLSL;
    case mlsdk_encoder_shader_type_hlsl:
        return ShaderType::HLSL;
    default:
        assert(false && "unknown shader type");
        return ShaderType::GLSL;
    }
}

std::vector<int64_t> to_vector(const int64_t *data, size_t size) {
    if (data == nullptr || size == 0) {
        return {};
    }
    return {data, data + size};
}

std::vector<uint32_t> to_vector(const uint32_t *data, size_t size) {
    if (data == nullptr || size == 0) {
        return {};
    }
    return {data, data + size};
}

std::vector<std::string> to_string_vector(const char **data, size_t size) {
    if (data == nullptr || size == 0) {
        return {};
    }

    std::vector<std::string> output;
    output.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        assert(data[i] != nullptr && "name is null");
        output.emplace_back(data[i]);
    }
    return output;
}

template <typename CRef, typename CppRef> std::vector<CppRef> to_ref_vector(const CRef *refs, size_t size) {
    if (refs == nullptr || size == 0) {
        return {};
    }

    std::vector<CppRef> output;
    output.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        output.push_back(CppRef{refs[i].reference});
    }
    return output;
}

mlsdk_encoder_module_ref to_c_ref(ModuleRef ref) { return {ref.reference}; }
mlsdk_encoder_resource_ref to_c_ref(ResourceRef ref) { return {ref.reference}; }
mlsdk_encoder_constant_ref to_c_ref(ConstantRef ref) { return {ref.reference}; }
mlsdk_encoder_binding_slot_ref to_c_ref(BindingSlotRef ref) { return {ref.reference}; }
mlsdk_encoder_descriptor_set_info_ref to_c_ref(DescriptorSetInfoRef ref) { return {ref.reference}; }
mlsdk_encoder_segment_info_ref to_c_ref(SegmentInfoRef ref) { return {ref.reference}; }
mlsdk_encoder_push_const_range_ref to_c_ref(PushConstRangeRef ref) { return {ref.reference}; }

} // namespace

mlsdk_encoder *mlsdk_encoder_create(mlsdk_encoder_vk_header_version vkHeaderVersion) {
    return new mlsdk_encoder(vkHeaderVersion);
}

void mlsdk_encoder_destroy(mlsdk_encoder *encoder) { delete encoder; }

mlsdk_encoder_module_ref mlsdk_encoder_add_spirv_module(mlsdk_encoder *encoder, mlsdk_encoder_module_type type,
                                                        const char *name, const char *entryPoint, const uint32_t *code,
                                                        size_t numWords) {
    assert(encoder != nullptr && "encoder is null");
    assert(name != nullptr && "name is null");
    assert(entryPoint != nullptr && "entryPoint is null");

    return to_c_ref(
        encoder->encoder->AddModule(convert_module_type(type), name, entryPoint, to_vector(code, numWords)));
}

mlsdk_encoder_module_ref mlsdk_encoder_add_source_module(mlsdk_encoder *encoder, mlsdk_encoder_module_type type,
                                                         const char *name, const char *entryPoint,
                                                         mlsdk_encoder_shader_type shaderType, const char *code) {
    assert(encoder != nullptr && "encoder is null");
    assert(name != nullptr && "name is null");
    assert(entryPoint != nullptr && "entryPoint is null");

    const std::string source = code == nullptr ? std::string{} : std::string{code};
    return to_c_ref(encoder->encoder->AddModule(convert_module_type(type), name, entryPoint,
                                                convert_shader_type(shaderType), source));
}

mlsdk_encoder_resource_ref mlsdk_encoder_add_input_resource(mlsdk_encoder *encoder,
                                                            mlsdk_encoder_vk_descriptor_type vkDescriptorType,
                                                            mlsdk_encoder_vk_format vkFormat, const int64_t *shape,
                                                            size_t shapeSize, const int64_t *strides,
                                                            size_t stridesSize) {
    assert(encoder != nullptr && "encoder is null");
    return to_c_ref(encoder->encoder->AddInputResource(static_cast<DescriptorType>(vkDescriptorType),
                                                       static_cast<FormatType>(vkFormat), to_vector(shape, shapeSize),
                                                       to_vector(strides, stridesSize)));
}

mlsdk_encoder_resource_ref mlsdk_encoder_add_input_resource_with_alias_group(
    mlsdk_encoder *encoder, mlsdk_encoder_vk_descriptor_type vkDescriptorType, mlsdk_encoder_vk_format vkFormat,
    const int64_t *shape, size_t shapeSize, const int64_t *strides, size_t stridesSize,
    mlsdk_encoder_alias_group_id aliasGroupId) {
    assert(encoder != nullptr && "encoder is null");
    return to_c_ref(encoder->encoder->AddInputResource(
        static_cast<DescriptorType>(vkDescriptorType), static_cast<FormatType>(vkFormat), to_vector(shape, shapeSize),
        to_vector(strides, stridesSize), static_cast<AliasGroupId>(aliasGroupId)));
}

mlsdk_encoder_resource_ref mlsdk_encoder_add_output_resource(mlsdk_encoder *encoder,
                                                             mlsdk_encoder_vk_descriptor_type vkDescriptorType,
                                                             mlsdk_encoder_vk_format vkFormat, const int64_t *shape,
                                                             size_t shapeSize, const int64_t *strides,
                                                             size_t stridesSize) {
    assert(encoder != nullptr && "encoder is null");
    return to_c_ref(encoder->encoder->AddOutputResource(static_cast<DescriptorType>(vkDescriptorType),
                                                        static_cast<FormatType>(vkFormat), to_vector(shape, shapeSize),
                                                        to_vector(strides, stridesSize)));
}

mlsdk_encoder_resource_ref mlsdk_encoder_add_output_resource_with_alias_group(
    mlsdk_encoder *encoder, mlsdk_encoder_vk_descriptor_type vkDescriptorType, mlsdk_encoder_vk_format vkFormat,
    const int64_t *shape, size_t shapeSize, const int64_t *strides, size_t stridesSize,
    mlsdk_encoder_alias_group_id aliasGroupId) {
    assert(encoder != nullptr && "encoder is null");
    return to_c_ref(encoder->encoder->AddOutputResource(
        static_cast<DescriptorType>(vkDescriptorType), static_cast<FormatType>(vkFormat), to_vector(shape, shapeSize),
        to_vector(strides, stridesSize), static_cast<AliasGroupId>(aliasGroupId)));
}

mlsdk_encoder_resource_ref mlsdk_encoder_add_intermediate_resource(mlsdk_encoder *encoder,
                                                                   mlsdk_encoder_vk_descriptor_type vkDescriptorType,
                                                                   mlsdk_encoder_vk_format vkFormat,
                                                                   const int64_t *shape, size_t shapeSize,
                                                                   const int64_t *strides, size_t stridesSize) {
    assert(encoder != nullptr && "encoder is null");
    return to_c_ref(encoder->encoder->AddIntermediateResource(
        static_cast<DescriptorType>(vkDescriptorType), static_cast<FormatType>(vkFormat), to_vector(shape, shapeSize),
        to_vector(strides, stridesSize)));
}

mlsdk_encoder_resource_ref mlsdk_encoder_add_intermediate_resource_with_alias_group(
    mlsdk_encoder *encoder, mlsdk_encoder_vk_descriptor_type vkDescriptorType, mlsdk_encoder_vk_format vkFormat,
    const int64_t *shape, size_t shapeSize, const int64_t *strides, size_t stridesSize,
    mlsdk_encoder_alias_group_id aliasGroupId) {
    assert(encoder != nullptr && "encoder is null");
    return to_c_ref(encoder->encoder->AddIntermediateResource(
        static_cast<DescriptorType>(vkDescriptorType), static_cast<FormatType>(vkFormat), to_vector(shape, shapeSize),
        to_vector(strides, stridesSize), static_cast<AliasGroupId>(aliasGroupId)));
}

mlsdk_encoder_resource_ref mlsdk_encoder_add_constant_resource(mlsdk_encoder *encoder, mlsdk_encoder_vk_format vkFormat,
                                                               const int64_t *shape, size_t shapeSize,
                                                               const int64_t *strides, size_t stridesSize) {
    assert(encoder != nullptr && "encoder is null");
    return to_c_ref(encoder->encoder->AddConstantResource(
        static_cast<FormatType>(vkFormat), to_vector(shape, shapeSize), to_vector(strides, stridesSize)));
}

void mlsdk_encoder_add_sampler_config(mlsdk_encoder *encoder, mlsdk_encoder_resource_ref resource,
                                      uint32_t samplerMinFilter, uint32_t samplerMagFilter,
                                      uint32_t samplerAddressModeU, uint32_t samplerAddressModeV,
                                      uint32_t samplerBorderColor) {
    assert(encoder != nullptr && "encoder is null");
    encoder->encoder->AddSamplerConfig(ResourceRef{resource.reference}, samplerMinFilter, samplerMagFilter,
                                       samplerAddressModeU, samplerAddressModeV, samplerBorderColor);
}

void mlsdk_encoder_set_alias_group(mlsdk_encoder *encoder, mlsdk_encoder_resource_ref resource,
                                   mlsdk_encoder_alias_group_id aliasGroupId) {
    assert(encoder != nullptr && "encoder is null");
    encoder->encoder->SetAliasGroup(ResourceRef{resource.reference}, static_cast<AliasGroupId>(aliasGroupId));
}

mlsdk_encoder_constant_ref mlsdk_encoder_add_constant(mlsdk_encoder *encoder, mlsdk_encoder_resource_ref resource,
                                                      const void *data, size_t sizeInBytes, int64_t sparsityDimension) {
    assert(encoder != nullptr && "encoder is null");
    return to_c_ref(
        encoder->encoder->AddConstant(ResourceRef{resource.reference}, data, sizeInBytes, sparsityDimension));
}

mlsdk_encoder_binding_slot_ref mlsdk_encoder_add_binding_slot(mlsdk_encoder *encoder, uint32_t binding,
                                                              mlsdk_encoder_resource_ref resource) {
    assert(encoder != nullptr && "encoder is null");
    return to_c_ref(encoder->encoder->AddBindingSlot(binding, ResourceRef{resource.reference}));
}

mlsdk_encoder_descriptor_set_info_ref
mlsdk_encoder_add_descriptor_set_info(mlsdk_encoder *encoder, const mlsdk_encoder_binding_slot_ref *bindings,
                                      size_t numBindings, uint32_t setIndex) {
    assert(encoder != nullptr && "encoder is null");
    return to_c_ref(encoder->encoder->AddDescriptorSetInfo(
        to_ref_vector<mlsdk_encoder_binding_slot_ref, BindingSlotRef>(bindings, numBindings), setIndex));
}

mlsdk_encoder_push_const_range_ref mlsdk_encoder_add_push_const_range(mlsdk_encoder *encoder, uint32_t stageFlags,
                                                                      uint32_t offset, uint32_t size) {
    assert(encoder != nullptr && "encoder is null");
    return to_c_ref(encoder->encoder->AddPushConstRange(stageFlags, offset, size));
}

mlsdk_encoder_segment_info_ref
mlsdk_encoder_add_segment_info(mlsdk_encoder *encoder, mlsdk_encoder_module_ref module, const char *name,
                               const mlsdk_encoder_descriptor_set_info_ref *descriptors, size_t numDescriptors,
                               const mlsdk_encoder_binding_slot_ref *inputs, size_t numInputs,
                               const mlsdk_encoder_binding_slot_ref *outputs, size_t numOutputs,
                               const mlsdk_encoder_constant_ref *constants, size_t numConstants,
                               const uint32_t dispatchShape[3],
                               const mlsdk_encoder_push_const_range_ref *pushConstRanges, size_t numPushConstRanges) {
    assert(encoder != nullptr && "encoder is null");
    assert(name != nullptr && "name is null");

    std::array<uint32_t, 3> dispatch{};
    if (dispatchShape != nullptr) {
        dispatch = {dispatchShape[0], dispatchShape[1], dispatchShape[2]};
    }

    return to_c_ref(encoder->encoder->AddSegmentInfo(
        ModuleRef{module.reference}, name,
        to_ref_vector<mlsdk_encoder_descriptor_set_info_ref, DescriptorSetInfoRef>(descriptors, numDescriptors),
        to_ref_vector<mlsdk_encoder_binding_slot_ref, BindingSlotRef>(inputs, numInputs),
        to_ref_vector<mlsdk_encoder_binding_slot_ref, BindingSlotRef>(outputs, numOutputs),
        to_ref_vector<mlsdk_encoder_constant_ref, ConstantRef>(constants, numConstants), dispatch,
        to_ref_vector<mlsdk_encoder_push_const_range_ref, PushConstRangeRef>(pushConstRanges, numPushConstRanges)));
}

void mlsdk_encoder_add_model_sequence_inputs_outputs(mlsdk_encoder *encoder,
                                                     const mlsdk_encoder_binding_slot_ref *inputs, size_t numInputs,
                                                     const char **inputNames, size_t numInputNames,
                                                     const mlsdk_encoder_binding_slot_ref *outputs, size_t numOutputs,
                                                     const char **outputNames, size_t numOutputNames) {
    assert(encoder != nullptr && "encoder is null");
    encoder->encoder->AddModelSequenceInputsOutputs(
        to_ref_vector<mlsdk_encoder_binding_slot_ref, BindingSlotRef>(inputs, numInputs),
        to_string_vector(inputNames, numInputNames),
        to_ref_vector<mlsdk_encoder_binding_slot_ref, BindingSlotRef>(outputs, numOutputs),
        to_string_vector(outputNames, numOutputNames));
}

void mlsdk_encoder_finish(mlsdk_encoder *encoder) {
    assert(encoder != nullptr && "encoder is null");
    encoder->encoder->Finish();
}

bool mlsdk_encoder_write_to_file(mlsdk_encoder *encoder, const char *path) {
    assert(encoder != nullptr && "encoder is null");
    assert(path != nullptr && "path is null");

    std::ofstream output(path, std::ofstream::binary | std::ofstream::trunc);
    if (!output) {
        return false;
    }
    return encoder->encoder->WriteTo(output);
}
