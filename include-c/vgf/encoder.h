/*
 * SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ML_SDK_VGF_LIB_ENCODER_API_H
#define ML_SDK_VGF_LIB_ENCODER_API_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif          // __cplusplus
#ifdef __GNUC__ // GCC/CLANG
#    define MLSDK_EXPORT __attribute__((visibility("default")))
#    define MLSDK_IMPORT
#else
#    ifdef _MSC_VER
#        define MLSDK_EXPORT __declspec(dllexport)
#        define MLSDK_IMPORT __declspec(dllimport)
#    else
#        define MLSDK_EXPORT
#        define MLSDK_IMPORT
#        pragma warning Undefined dynamic library export / import semantic for detected toolchain.
#    endif
#endif
#ifdef MLSDK_DYNAMIC_LIB    // Building for a dynamic lib
#    ifdef MLSDK_EXPORT_API // Building the library for export
#        define MLSDKAPI MLSDK_EXPORT
#    else
#        define MLSDKAPI MLSDK_IMPORT
#    endif
#else
#    define MLSDKAPI
#endif

typedef struct mlsdk_encoder_s mlsdk_encoder;

/**
 * @brief Sparsity dimension value for constants that are not sparse.
 */
#define MLSDK_ENCODER_CONSTANT_NOT_SPARSE_DIMENSION (INT64_C(-1))

/**
 * @brief Descriptor set index sentinel for preserving legacy positional descriptor set semantics.
 */
#define MLSDK_ENCODER_DESCRIPTOR_SET_INDEX_UNSPECIFIED UINT32_MAX

/**
 * \defgroup VGFCEncoderAPI Encoder C API
 * @{
 */

/**
 * @brief Type for VK_HEADER_VERSION.
 */
typedef uint16_t mlsdk_encoder_vk_header_version;

/**
 * @brief Type for a VkDescriptorType enum value.
 */
typedef int32_t mlsdk_encoder_vk_descriptor_type;

/**
 * @brief Type for a VkFormat enum value.
 */
typedef int32_t mlsdk_encoder_vk_format;

/**
 * @brief Type for model resource alias group ids.
 */
typedef uint32_t mlsdk_encoder_alias_group_id;

/**
 * @brief Module type encoded in a VGF module table entry and model sequence segment.
 */
typedef enum {
    /** Compute pipeline module. */
    mlsdk_encoder_module_type_compute = 0,
    /** Graph pipeline module. */
    mlsdk_encoder_module_type_graph = 1,
} mlsdk_encoder_module_type;

/**
 * @brief Source language for non-SPIR-V shader modules.
 */
typedef enum {
    /** GLSL source module. */
    mlsdk_encoder_shader_type_glsl = 0,
    /** HLSL source module. */
    mlsdk_encoder_shader_type_hlsl = 1,
} mlsdk_encoder_shader_type;

/**
 * @brief Reference to an encoded module table entry.
 */
typedef struct {
    /** Zero-based module table index. */
    uint32_t reference;
} mlsdk_encoder_module_ref;

/**
 * @brief Reference to an encoded model resource table entry.
 */
typedef struct {
    /** Zero-based model resource table index. */
    uint32_t reference;
} mlsdk_encoder_resource_ref;

/**
 * @brief Reference to an encoded constant table entry.
 */
typedef struct {
    /** Zero-based constant table index. */
    uint32_t reference;
} mlsdk_encoder_constant_ref;

/**
 * @brief Reference to an encoded binding slot entry.
 */
typedef struct {
    /** Zero-based binding slot index in the encoder. */
    uint32_t reference;
} mlsdk_encoder_binding_slot_ref;

/**
 * @brief Reference to an encoded descriptor set info entry.
 */
typedef struct {
    /** Zero-based descriptor set info index in the encoder. */
    uint32_t reference;
} mlsdk_encoder_descriptor_set_info_ref;

/**
 * @brief Reference to an encoded segment info entry.
 */
typedef struct {
    /** Zero-based model sequence segment index. */
    uint32_t reference;
} mlsdk_encoder_segment_info_ref;

/**
 * @brief Reference to an encoded push constant range entry.
 */
typedef struct {
    /** Zero-based push constant range index in the encoder. */
    uint32_t reference;
} mlsdk_encoder_push_const_range_ref;

/**
 * @brief Creates a VGF encoder.
 *
 * @param vkHeaderVersion Value of VK_HEADER_VERSION used when encoding the VGF.
 * @return Encoder handle. Destroy with mlsdk_encoder_destroy.
 */
MLSDKAPI mlsdk_encoder *mlsdk_encoder_create(mlsdk_encoder_vk_header_version vkHeaderVersion);

/**
 * @brief Destroys a VGF encoder.
 *
 * @param encoder Encoder handle returned by mlsdk_encoder_create. Passing nullptr is allowed.
 */
MLSDKAPI void mlsdk_encoder_destroy(mlsdk_encoder *encoder);

/**
 * @brief Adds a SPIR-V module to the VGF.
 *
 * @param encoder Encoder handle.
 * @param type Module type.
 * @param name Module name.
 * @param entryPoint Module entry point.
 * @param code Pointer to SPIR-V words. May be nullptr when numWords is zero for a placeholder module.
 * @param numWords Number of SPIR-V words.
 * @return Reference to the added module.
 */
MLSDKAPI mlsdk_encoder_module_ref mlsdk_encoder_add_spirv_module(mlsdk_encoder *encoder, mlsdk_encoder_module_type type,
                                                                 const char *name, const char *entryPoint,
                                                                 const uint32_t *code, size_t numWords);

/**
 * @brief Adds a source-code module to the VGF.
 *
 * @param encoder Encoder handle.
 * @param type Module type.
 * @param name Module name.
 * @param entryPoint Module entry point.
 * @param shaderType Shader source language.
 * @param code Null-terminated shader source. May be nullptr for an empty module.
 * @return Reference to the added module.
 */
MLSDKAPI mlsdk_encoder_module_ref mlsdk_encoder_add_source_module(mlsdk_encoder *encoder,
                                                                  mlsdk_encoder_module_type type, const char *name,
                                                                  const char *entryPoint,
                                                                  mlsdk_encoder_shader_type shaderType,
                                                                  const char *code);

/**
 * @brief Adds an input resource to the model resource table.
 *
 * @param encoder Encoder handle.
 * @param vkDescriptorType VkDescriptorType value for this resource.
 * @param vkFormat VkFormat value for this resource.
 * @param shape Pointer to tensor shape dimensions. May be nullptr when shapeSize is zero.
 * @param shapeSize Number of shape dimensions.
 * @param strides Pointer to tensor stride dimensions. May be nullptr when stridesSize is zero.
 * @param stridesSize Number of stride dimensions.
 * @return Reference to the added resource.
 */
MLSDKAPI mlsdk_encoder_resource_ref mlsdk_encoder_add_input_resource(mlsdk_encoder *encoder,
                                                                     mlsdk_encoder_vk_descriptor_type vkDescriptorType,
                                                                     mlsdk_encoder_vk_format vkFormat,
                                                                     const int64_t *shape, size_t shapeSize,
                                                                     const int64_t *strides, size_t stridesSize);

/**
 * @brief Adds an input resource with an alias group to the model resource table.
 *
 * @param encoder Encoder handle.
 * @param vkDescriptorType VkDescriptorType value for this resource.
 * @param vkFormat VkFormat value for this resource.
 * @param shape Pointer to tensor shape dimensions. May be nullptr when shapeSize is zero.
 * @param shapeSize Number of shape dimensions.
 * @param strides Pointer to tensor stride dimensions. May be nullptr when stridesSize is zero.
 * @param stridesSize Number of stride dimensions.
 * @param aliasGroupId Alias group shared with peer model resource table entries.
 * @return Reference to the added resource.
 */
MLSDKAPI mlsdk_encoder_resource_ref mlsdk_encoder_add_input_resource_with_alias_group(
    mlsdk_encoder *encoder, mlsdk_encoder_vk_descriptor_type vkDescriptorType, mlsdk_encoder_vk_format vkFormat,
    const int64_t *shape, size_t shapeSize, const int64_t *strides, size_t stridesSize,
    mlsdk_encoder_alias_group_id aliasGroupId);

/**
 * @brief Adds an output resource to the model resource table.
 *
 * @param encoder Encoder handle.
 * @param vkDescriptorType VkDescriptorType value for this resource.
 * @param vkFormat VkFormat value for this resource.
 * @param shape Pointer to tensor shape dimensions. May be nullptr when shapeSize is zero.
 * @param shapeSize Number of shape dimensions.
 * @param strides Pointer to tensor stride dimensions. May be nullptr when stridesSize is zero.
 * @param stridesSize Number of stride dimensions.
 * @return Reference to the added resource.
 */
MLSDKAPI mlsdk_encoder_resource_ref mlsdk_encoder_add_output_resource(mlsdk_encoder *encoder,
                                                                      mlsdk_encoder_vk_descriptor_type vkDescriptorType,
                                                                      mlsdk_encoder_vk_format vkFormat,
                                                                      const int64_t *shape, size_t shapeSize,
                                                                      const int64_t *strides, size_t stridesSize);

/**
 * @brief Adds an output resource with an alias group to the model resource table.
 *
 * @param encoder Encoder handle.
 * @param vkDescriptorType VkDescriptorType value for this resource.
 * @param vkFormat VkFormat value for this resource.
 * @param shape Pointer to tensor shape dimensions. May be nullptr when shapeSize is zero.
 * @param shapeSize Number of shape dimensions.
 * @param strides Pointer to tensor stride dimensions. May be nullptr when stridesSize is zero.
 * @param stridesSize Number of stride dimensions.
 * @param aliasGroupId Alias group shared with peer model resource table entries.
 * @return Reference to the added resource.
 */
MLSDKAPI mlsdk_encoder_resource_ref mlsdk_encoder_add_output_resource_with_alias_group(
    mlsdk_encoder *encoder, mlsdk_encoder_vk_descriptor_type vkDescriptorType, mlsdk_encoder_vk_format vkFormat,
    const int64_t *shape, size_t shapeSize, const int64_t *strides, size_t stridesSize,
    mlsdk_encoder_alias_group_id aliasGroupId);

/**
 * @brief Adds an intermediate resource to the model resource table.
 *
 * @param encoder Encoder handle.
 * @param vkDescriptorType VkDescriptorType value for this resource.
 * @param vkFormat VkFormat value for this resource.
 * @param shape Pointer to tensor shape dimensions. May be nullptr when shapeSize is zero.
 * @param shapeSize Number of shape dimensions.
 * @param strides Pointer to tensor stride dimensions. May be nullptr when stridesSize is zero.
 * @param stridesSize Number of stride dimensions.
 * @return Reference to the added resource.
 */
MLSDKAPI mlsdk_encoder_resource_ref mlsdk_encoder_add_intermediate_resource(
    mlsdk_encoder *encoder, mlsdk_encoder_vk_descriptor_type vkDescriptorType, mlsdk_encoder_vk_format vkFormat,
    const int64_t *shape, size_t shapeSize, const int64_t *strides, size_t stridesSize);

/**
 * @brief Adds an intermediate resource with an alias group to the model resource table.
 *
 * @param encoder Encoder handle.
 * @param vkDescriptorType VkDescriptorType value for this resource.
 * @param vkFormat VkFormat value for this resource.
 * @param shape Pointer to tensor shape dimensions. May be nullptr when shapeSize is zero.
 * @param shapeSize Number of shape dimensions.
 * @param strides Pointer to tensor stride dimensions. May be nullptr when stridesSize is zero.
 * @param stridesSize Number of stride dimensions.
 * @param aliasGroupId Alias group shared with peer model resource table entries.
 * @return Reference to the added resource.
 */
MLSDKAPI mlsdk_encoder_resource_ref mlsdk_encoder_add_intermediate_resource_with_alias_group(
    mlsdk_encoder *encoder, mlsdk_encoder_vk_descriptor_type vkDescriptorType, mlsdk_encoder_vk_format vkFormat,
    const int64_t *shape, size_t shapeSize, const int64_t *strides, size_t stridesSize,
    mlsdk_encoder_alias_group_id aliasGroupId);

/**
 * @brief Adds a constant resource to the model resource table.
 *
 * @param encoder Encoder handle.
 * @param vkFormat VkFormat value for this resource.
 * @param shape Pointer to tensor shape dimensions. May be nullptr when shapeSize is zero.
 * @param shapeSize Number of shape dimensions.
 * @param strides Pointer to tensor stride dimensions. May be nullptr when stridesSize is zero.
 * @param stridesSize Number of stride dimensions.
 * @return Reference to the added resource.
 */
MLSDKAPI mlsdk_encoder_resource_ref mlsdk_encoder_add_constant_resource(mlsdk_encoder *encoder,
                                                                        mlsdk_encoder_vk_format vkFormat,
                                                                        const int64_t *shape, size_t shapeSize,
                                                                        const int64_t *strides, size_t stridesSize);

/**
 * @brief Sets sampler metadata for an existing model resource table entry.
 *
 * @param encoder Encoder handle.
 * @param resource Reference to the resource to update.
 * @param samplerMinFilter VkFilter value used for minification.
 * @param samplerMagFilter VkFilter value used for magnification.
 * @param samplerAddressModeU VkSamplerAddressMode for the U axis.
 * @param samplerAddressModeV VkSamplerAddressMode for the V axis.
 * @param samplerBorderColor VkBorderColor value.
 */
MLSDKAPI void mlsdk_encoder_add_sampler_config(mlsdk_encoder *encoder, mlsdk_encoder_resource_ref resource,
                                               uint32_t samplerMinFilter, uint32_t samplerMagFilter,
                                               uint32_t samplerAddressModeU, uint32_t samplerAddressModeV,
                                               uint32_t samplerBorderColor);

/**
 * @brief Assigns an alias group to an existing non-constant model resource table entry.
 *
 * @param encoder Encoder handle.
 * @param resource Reference to the resource to update.
 * @param aliasGroupId Alias group shared with peer model resource table entries.
 */
MLSDKAPI void mlsdk_encoder_set_alias_group(mlsdk_encoder *encoder, mlsdk_encoder_resource_ref resource,
                                            mlsdk_encoder_alias_group_id aliasGroupId);

/**
 * @brief Adds constant bytes for a constant resource.
 *
 * @param encoder Encoder handle.
 * @param resource Reference to a constant resource in the model resource table.
 * @param data Pointer to constant data bytes.
 * @param sizeInBytes Size of constant data in bytes.
 * @param sparsityDimension Sparse dimension, or MLSDK_ENCODER_CONSTANT_NOT_SPARSE_DIMENSION for non-sparse constants.
 * @return Reference to the added constant.
 */
MLSDKAPI mlsdk_encoder_constant_ref mlsdk_encoder_add_constant(mlsdk_encoder *encoder,
                                                               mlsdk_encoder_resource_ref resource, const void *data,
                                                               size_t sizeInBytes, int64_t sparsityDimension);

/**
 * @brief Adds a binding slot associated with a model resource table entry.
 *
 * @param encoder Encoder handle.
 * @param binding Binding number exposed to the shader.
 * @param resource Reference to the resource bound at this slot.
 * @return Reference to the added binding slot.
 */
MLSDKAPI mlsdk_encoder_binding_slot_ref mlsdk_encoder_add_binding_slot(mlsdk_encoder *encoder, uint32_t binding,
                                                                       mlsdk_encoder_resource_ref resource);

/**
 * @brief Adds descriptor set information for a group of binding slots.
 *
 * @param encoder Encoder handle.
 * @param bindings Pointer to binding slot references. May be nullptr when numBindings is zero.
 * @param numBindings Number of binding slot references.
 * @param setIndex Explicit descriptor set index, or MLSDK_ENCODER_DESCRIPTOR_SET_INDEX_UNSPECIFIED to use legacy
 * positional semantics.
 * @return Reference to the added descriptor set info.
 */
MLSDKAPI mlsdk_encoder_descriptor_set_info_ref mlsdk_encoder_add_descriptor_set_info(
    mlsdk_encoder *encoder, const mlsdk_encoder_binding_slot_ref *bindings, size_t numBindings, uint32_t setIndex);

/**
 * @brief Adds a push constant range.
 *
 * @param encoder Encoder handle.
 * @param stageFlags Shader stage flags for the range.
 * @param offset Start offset in bytes.
 * @param size Size in bytes.
 * @return Reference to the added push constant range.
 */
MLSDKAPI mlsdk_encoder_push_const_range_ref mlsdk_encoder_add_push_const_range(mlsdk_encoder *encoder,
                                                                               uint32_t stageFlags, uint32_t offset,
                                                                               uint32_t size);

/**
 * @brief Adds a model sequence segment.
 *
 * @param encoder Encoder handle.
 * @param module Module used by this segment.
 * @param name Segment name.
 * @param descriptors Pointer to descriptor set info references. May be nullptr when numDescriptors is zero.
 * @param numDescriptors Number of descriptor set info references.
 * @param inputs Pointer to segment input binding slot references. May be nullptr when numInputs is zero.
 * @param numInputs Number of input binding slot references.
 * @param outputs Pointer to segment output binding slot references. May be nullptr when numOutputs is zero.
 * @param numOutputs Number of output binding slot references.
 * @param constants Pointer to segment constant references. May be nullptr when numConstants is zero.
 * @param numConstants Number of constant references.
 * @param dispatchShape Three-dimensional dispatch shape. Passing nullptr encodes {0, 0, 0}.
 * @param pushConstRanges Pointer to push constant range references. May be nullptr when numPushConstRanges is zero.
 * @param numPushConstRanges Number of push constant range references.
 * @return Reference to the added segment.
 */
MLSDKAPI mlsdk_encoder_segment_info_ref mlsdk_encoder_add_segment_info(
    mlsdk_encoder *encoder, mlsdk_encoder_module_ref module, const char *name,
    const mlsdk_encoder_descriptor_set_info_ref *descriptors, size_t numDescriptors,
    const mlsdk_encoder_binding_slot_ref *inputs, size_t numInputs, const mlsdk_encoder_binding_slot_ref *outputs,
    size_t numOutputs, const mlsdk_encoder_constant_ref *constants, size_t numConstants,
    const uint32_t dispatchShape[3], const mlsdk_encoder_push_const_range_ref *pushConstRanges,
    size_t numPushConstRanges);

/**
 * @brief Adds model-level input and output binding slot sequences.
 *
 * @param encoder Encoder handle.
 * @param inputs Pointer to model input binding slot references. May be nullptr when numInputs is zero.
 * @param numInputs Number of model input binding slot references.
 * @param inputNames Pointer to null-terminated input names. May be nullptr when numInputNames is zero.
 * @param numInputNames Number of input names. Must be zero or equal to numInputs.
 * @param outputs Pointer to model output binding slot references. May be nullptr when numOutputs is zero.
 * @param numOutputs Number of model output binding slot references.
 * @param outputNames Pointer to null-terminated output names. May be nullptr when numOutputNames is zero.
 * @param numOutputNames Number of output names. Must be zero or equal to numOutputs.
 */
MLSDKAPI void
mlsdk_encoder_add_model_sequence_inputs_outputs(mlsdk_encoder *encoder, const mlsdk_encoder_binding_slot_ref *inputs,
                                                size_t numInputs, const char **inputNames, size_t numInputNames,
                                                const mlsdk_encoder_binding_slot_ref *outputs, size_t numOutputs,
                                                const char **outputNames, size_t numOutputNames);

/**
 * @brief Finishes encoding after all VGF contents have been added.
 *
 * @param encoder Encoder handle.
 */
MLSDKAPI void mlsdk_encoder_finish(mlsdk_encoder *encoder);

/**
 * @brief Writes a finished VGF encoder to a file.
 *
 * @param encoder Encoder handle.
 * @param path Output file path.
 * @return true when writing succeeded, false otherwise.
 */
MLSDKAPI bool mlsdk_encoder_write_to_file(mlsdk_encoder *encoder, const char *path);

/**@}*/

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // ML_SDK_VGF_LIB_ENCODER_API_H
