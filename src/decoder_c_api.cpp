/*
 * SPDX-FileCopyrightText: Copyright 2023-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf/decoder.h"
#include "vgf/decoder.hpp"

#include "header.hpp"

#include <cassert>

using namespace mlsdk::vgflib;

void mlsdk_decoder_get_version(mlsdk_decoder_vgf_version *version) {
    assert(version != nullptr && "version is null");
    version->major = HEADER_MAJOR_VERSION_VALUE;
    version->minor = HEADER_MINOR_VERSION_VALUE;
    version->patch = HEADER_PATCH_VERSION_VALUE;
}

size_t mlsdk_decoder_header_size() { return HeaderSize(); }

size_t mlsdk_decoder_header_decoder_mem_reqs() { return HeaderDecoderSize(); }

mlsdk_decoder_header_decoder *mlsdk_decoder_create_header_decoder(const void *const headerData, void *decoderMemory) {
    assert(headerData != nullptr && "headerData is null");
    assert(decoderMemory != nullptr && "decoderMemory is null");
    return reinterpret_cast<mlsdk_decoder_header_decoder *>(CreateHeaderDecoderInPlace(headerData, decoderMemory));
}

bool mlsdk_decoder_is_latest_version(const mlsdk_decoder_header_decoder *const decoder) {
    assert(decoder != nullptr && "decoder is null");
    return reinterpret_cast<const HeaderDecoder *>(decoder)->IsLatestVersion();
}

bool mlsdk_decoder_is_header_valid(const mlsdk_decoder_header_decoder *const decoder) {
    assert(decoder != nullptr && "decoder is null");
    return reinterpret_cast<const HeaderDecoder *>(decoder)->IsValid();
}

bool mlsdk_decoder_is_header_compatible(const mlsdk_decoder_header_decoder *const decoder) {
    assert(decoder != nullptr && "decoder is null");
    return reinterpret_cast<const HeaderDecoder *>(decoder)->CheckVersion();
}

void mlsdk_decoder_get_header_version(const mlsdk_decoder_header_decoder *const decoder,
                                      mlsdk_decoder_vgf_version *version) {
    assert(decoder != nullptr && "decoder is null");
    assert(version != nullptr && "version is null");
    const auto *const d = reinterpret_cast<const HeaderDecoder *>(decoder);
    version->major = d->GetMajor();
    version->minor = d->GetMinor();
    version->patch = d->GetPatch();
}

void mlsdk_decoder_get_encoder_vk_header_version(const mlsdk_decoder_header_decoder *const decoder,
                                                 mlsdk_vk_header_version *vkHeaderVersion) {
    assert(decoder != nullptr && "decoder is null");
    const auto *const d = reinterpret_cast<const HeaderDecoder *>(decoder);
    *vkHeaderVersion = d->GetEncoderVulkanHeadersVersion();
}

void mlsdk_decoder_get_header_section_info(const mlsdk_decoder_header_decoder *const decoder,
                                           mlsdk_decoder_section sectionName, mlsdk_decoder_vgf_section_info *section) {
    assert(decoder != nullptr && "decoder is null");
    assert(section != nullptr && "sections is null");
    const auto *const d = reinterpret_cast<const HeaderDecoder *>(decoder);
    switch (sectionName) {
    case mlsdk_decoder_section_modules:
        section->offset = d->GetModuleTableOffset();
        section->size = d->GetModuleTableSize();
        break;
    case mlsdk_decoder_section_model_sequence:
        section->offset = d->GetModelSequenceTableOffset();
        section->size = d->GetModelSequenceTableSize();
        break;
    case mlsdk_decoder_section_resources:
        section->offset = d->GetModelResourceTableOffset();
        section->size = d->GetModelResourceTableSize();
        break;
    case mlsdk_decoder_section_constants:
        section->offset = d->GetConstantsOffset();
        section->size = d->GetConstantsSize();
        break;
    default:
        assert(false && "Unknown section name");
    }
}

size_t mlsdk_decoder_module_table_decoder_mem_reqs() { return ModuleTableDecoderSize(); }

bool mlsdk_decoder_is_valid_module_table(const void *moduleTableData, const uint64_t size) {
    assert(moduleTableData != nullptr && "moduleTableData is null");
    return VerifyModuleTable(moduleTableData, size);
}

mlsdk_decoder_module_table_decoder *mlsdk_decoder_create_module_table_decoder(const void *const moduleTableData,
                                                                              void *decoderMemory) {
    assert(moduleTableData != nullptr && "moduleTableData is null");
    assert(decoderMemory != nullptr && "decoderMemory is null");
    return reinterpret_cast<mlsdk_decoder_module_table_decoder *>(
        CreateModuleTableDecoderInPlace(moduleTableData, decoderMemory));
}

size_t mlsdk_decoder_get_module_table_num_entries(const mlsdk_decoder_module_table_decoder *const decoder) {
    assert(decoder != nullptr && "decoder is null");
    return reinterpret_cast<const ModuleTableDecoder *>(decoder)->size();
}

namespace {
inline mlsdk_decoder_module_type convert_module_type(ModuleType type) {
    switch (type) {
    case ModuleType::COMPUTE:
        return mlsdk_decoder_module_type_compute;
    case ModuleType::GRAPH:
        return mlsdk_decoder_module_type_graph;
    default:
        assert(false && "unknow type");
        return mlsdk_decoder_module_type_compute;
    }
}

inline mlsdk_vk_descriptor_type_optional convert_descriptor_type(std::optional<DescriptorType> type) {
    mlsdk_vk_descriptor_type_optional desc_type_optional;
    memset(&desc_type_optional, 0, sizeof(mlsdk_vk_descriptor_type_optional));
    if (type.has_value()) {
        desc_type_optional.value = static_cast<mlsdk_vk_descriptor_type>(*type);
        desc_type_optional.has_value = true;
    }

    return desc_type_optional;
}

inline mlsdk_vk_format convert_vk_format(FormatType format) {
    assert(UndefinedFormat() == mlsdk_vk_format_undefined() && "API vk_format_undefined mismatch");

    return static_cast<mlsdk_vk_format>(format);
}
} // namespace

mlsdk_decoder_module_type mlsdk_decoder_get_module_type(const mlsdk_decoder_module_table_decoder *const decoder,
                                                        uint32_t idx) {
    assert(decoder != nullptr && "decoder is null");
    auto type = reinterpret_cast<const ModuleTableDecoder *>(decoder)->getModuleType(idx);
    return convert_module_type(type);
}

const char *mlsdk_decoder_get_module_name(const mlsdk_decoder_module_table_decoder *const decoder, uint32_t idx) {
    assert(decoder != nullptr && "decoder is null");
    return reinterpret_cast<const ModuleTableDecoder *>(decoder)->getModuleName(idx).data();
}

const char *mlsdk_decoder_get_module_entry_point(const mlsdk_decoder_module_table_decoder *const decoder,
                                                 uint32_t idx) {
    assert(decoder != nullptr && "decoder is null");
    return reinterpret_cast<const ModuleTableDecoder *>(decoder)->getModuleEntryPoint(idx).data();
}

void mlsdk_decoder_get_module_code(const mlsdk_decoder_module_table_decoder *const decoder, uint32_t idx,
                                   mlsdk_decoder_spirv_code *spirvCode) {
    assert(decoder != nullptr && "decoder is null");
    assert(spirvCode != nullptr && "spirvCode is null");
    DataView<uint32_t> view = reinterpret_cast<const ModuleTableDecoder *>(decoder)->getModuleCode(idx);
    spirvCode->code = view.begin();
    spirvCode->words = view.size();
}

/**********************************************************************************************************************/
namespace {
mlsdk_decoder_binding_slots_handle to_c_handle(BindingSlotArrayHandle handleIn) {
    return reinterpret_cast<mlsdk_decoder_binding_slots_handle>(handleIn);
}

BindingSlotArrayHandle from_c_handle(mlsdk_decoder_binding_slots_handle handleIn) {
    return reinterpret_cast<BindingSlotArrayHandle>(handleIn);
}

mlsdk_decoder_push_constant_ranges_handle to_c_handle(PushConstantRangeHandle handleIn) {
    return reinterpret_cast<mlsdk_decoder_push_constant_ranges_handle>(handleIn);
}

PushConstantRangeHandle from_c_handle(mlsdk_decoder_push_constant_ranges_handle handleIn) {
    return reinterpret_cast<PushConstantRangeHandle>(handleIn);
}

} // namespace

size_t mlsdk_decoder_binding_slot_size(const mlsdk_decoder_model_sequence_decoder *const modelSequenceDecoder,
                                       mlsdk_decoder_binding_slots_handle handle) {
    assert(modelSequenceDecoder != nullptr && "modelSequenceDecoder is null");
    return reinterpret_cast<const ModelSequenceTableDecoder *>(modelSequenceDecoder)
        ->getBindingsSize(from_c_handle(handle));
}

uint32_t mlsdk_decoder_binding_slot_binding_id(const mlsdk_decoder_model_sequence_decoder *const modelSequenceDecoder,
                                               mlsdk_decoder_binding_slots_handle handle, uint32_t slotIdx) {
    assert(modelSequenceDecoder != nullptr && "modelSequenceDecoder is null");
    return reinterpret_cast<const ModelSequenceTableDecoder *>(modelSequenceDecoder)
        ->getBindingSlotBinding(from_c_handle(handle), slotIdx);
}

uint32_t mlsdk_decoder_binding_slot_mrt_index(const mlsdk_decoder_model_sequence_decoder *const modelSequenceDecoder,
                                              mlsdk_decoder_binding_slots_handle handle, uint32_t slotIdx) {
    assert(modelSequenceDecoder != nullptr && "modelSequenceDecoder is null");
    return reinterpret_cast<const ModelSequenceTableDecoder *>(modelSequenceDecoder)
        ->getBindingSlotMrtIndex(from_c_handle(handle), slotIdx);
}

/**********************************************************************************************************************/

size_t
mlsdk_decoder_get_push_constant_ranges_size(const mlsdk_decoder_model_sequence_decoder *const modelSequenceDecoder,
                                            mlsdk_decoder_push_constant_ranges_handle handle) {
    assert(modelSequenceDecoder != nullptr && "modelSequenceDecoder is null");
    return reinterpret_cast<const ModelSequenceTableDecoder *>(modelSequenceDecoder)
        ->getPushConstRangesSize(from_c_handle(handle));
}

uint32_t mlsdk_decoder_get_push_constant_range_stage_flags(
    const mlsdk_decoder_model_sequence_decoder *const modelSequenceDecoder,
    mlsdk_decoder_push_constant_ranges_handle handle, uint32_t rangeIdx) {
    assert(modelSequenceDecoder != nullptr && "modelSequenceDecoder is null");
    return reinterpret_cast<const ModelSequenceTableDecoder *>(modelSequenceDecoder)
        ->getPushConstRangeStageFlags(from_c_handle(handle), rangeIdx);
}

uint32_t
mlsdk_decoder_get_push_constant_range_offset(const mlsdk_decoder_model_sequence_decoder *const modelSequenceDecoder,
                                             mlsdk_decoder_push_constant_ranges_handle handle, uint32_t rangeIdx) {
    assert(modelSequenceDecoder != nullptr && "modelSequenceDecoder is null");
    return reinterpret_cast<const ModelSequenceTableDecoder *>(modelSequenceDecoder)
        ->getPushConstRangeOffset(from_c_handle(handle), rangeIdx);
}

uint32_t
mlsdk_decoder_get_push_constant_range_size(const mlsdk_decoder_model_sequence_decoder *const modelSequenceDecoder,
                                           mlsdk_decoder_push_constant_ranges_handle handle, uint32_t rangeIdx) {
    assert(modelSequenceDecoder != nullptr && "modelSequenceDecoder is null");
    return reinterpret_cast<const ModelSequenceTableDecoder *>(modelSequenceDecoder)
        ->getPushConstRangeSize(from_c_handle(handle), rangeIdx);
}

/**********************************************************************************************************************/

bool mlsdk_decoder_is_valid_model_sequence(const void *modelSequenceData, const uint64_t size) {
    assert(modelSequenceData != nullptr && "modelSequenceData is null");
    return VerifyModelSequenceTable(modelSequenceData, size);
}

mlsdk_decoder_model_sequence_decoder *mlsdk_decoder_create_model_sequence_decoder(const void *const modelSequenceData,
                                                                                  void *modelSequenceDecoderMemory) {
    assert(modelSequenceData != nullptr && "modelSequenceData is null");
    assert(modelSequenceDecoderMemory != nullptr && "modelSequenceDecoderMemory is null");
    return reinterpret_cast<mlsdk_decoder_model_sequence_decoder *>(
        CreateModelSequenceTableDecoderInPlace(modelSequenceData, modelSequenceDecoderMemory));
}

size_t mlsdk_decoder_model_sequence_decoder_mem_reqs() { return ModelResourceTableDecoderSize(); }

size_t
mlsdk_decoder_get_model_sequence_table_size(const mlsdk_decoder_model_sequence_decoder *const modelSequenceDecoder) {
    assert(modelSequenceDecoder != nullptr && "modelSequenceDecoder is null");
    return reinterpret_cast<const ModelSequenceTableDecoder *>(modelSequenceDecoder)->modelSequenceTableSize();
}

size_t mlsdk_decoder_model_sequence_get_segment_descriptorset_info_size(
    const mlsdk_decoder_model_sequence_decoder *const modelSequenceDecoder, uint32_t segIdx) {
    assert(modelSequenceDecoder != nullptr && "modelSequenceDecoder is null");
    return static_cast<mlsdk_decoder_module_type>(
        reinterpret_cast<const ModelSequenceTableDecoder *>(modelSequenceDecoder)
            ->getSegmentDescriptorSetInfosSize(segIdx));
}

void mlsdk_decoder_model_sequence_get_segment_constant_indexes(
    const mlsdk_decoder_model_sequence_decoder *const modelSequenceDecoder, uint32_t segIdx,
    mlsdk_decoder_constant_indexes *constant) {
    assert(modelSequenceDecoder != nullptr && "modelSequenceDecoder is null");
    assert(constant != nullptr && "constant is null");
    DataView<uint32_t> data =
        reinterpret_cast<const ModelSequenceTableDecoder *>(modelSequenceDecoder)->getSegmentConstantIndexes(segIdx);
    constant->data = data.begin();
    constant->size = data.size();
}

mlsdk_decoder_module_type
mlsdk_decoder_model_sequence_get_segment_type(const mlsdk_decoder_model_sequence_decoder *const modelSequenceDecoder,
                                              uint32_t segIdx) {
    assert(modelSequenceDecoder != nullptr && "modelSequenceDecoder is null");
    return static_cast<mlsdk_decoder_module_type>(
        reinterpret_cast<const ModelSequenceTableDecoder *>(modelSequenceDecoder)->getSegmentType(segIdx));
}

const char *
mlsdk_decoder_model_sequence_get_segment_name(const mlsdk_decoder_model_sequence_decoder *const modelSequenceDecoder,
                                              uint32_t segIdx) {
    assert(modelSequenceDecoder != nullptr && "modelSequenceDecoder is null");
    return reinterpret_cast<const ModelSequenceTableDecoder *>(modelSequenceDecoder)->getSegmentName(segIdx).data();
}

uint32_t mlsdk_decoder_model_sequence_get_segment_module_index(
    const mlsdk_decoder_model_sequence_decoder *const modelSequenceDecoder, uint32_t segIdx) {
    assert(modelSequenceDecoder != nullptr && "modelSequenceDecoder is null");
    return reinterpret_cast<const ModelSequenceTableDecoder *>(modelSequenceDecoder)->getSegmentModuleIndex(segIdx);
}

void mlsdk_decoder_model_sequence_get_segment_dispatch_shape(
    mlsdk_decoder_model_sequence_decoder *const modelSequenceDecoder, uint32_t segIdx,
    mlsdk_decoder_dispatch_shape *dispatchShape) {
    assert(modelSequenceDecoder != nullptr && "modelSequenceDecoder is null");
    assert(dispatchShape != nullptr && "dispatchShape is null");
    DataView<uint32_t> data =
        reinterpret_cast<ModelSequenceTableDecoder *>(modelSequenceDecoder)->getSegmentDispatchShape(segIdx);
    assert(data.size() == 3 && "wrong dispatchShape size");
    dispatchShape->data[0] = data[0];
    dispatchShape->data[1] = data[1];
    dispatchShape->data[2] = data[2];
}

mlsdk_decoder_push_constant_ranges_handle mlsdk_decoder_model_sequence_get_segment_push_constant_range(
    mlsdk_decoder_model_sequence_decoder *const modelSequenceDecoder, uint32_t segIdx) {
    assert(modelSequenceDecoder != nullptr && "modelSequenceDecoder is null");
    return to_c_handle(
        reinterpret_cast<ModelSequenceTableDecoder *>(modelSequenceDecoder)->getSegmentPushConstRange(segIdx));
}

mlsdk_decoder_binding_slots_handle mlsdk_decoder_model_sequence_get_segment_descriptor_binding_slot(
    mlsdk_decoder_model_sequence_decoder *const modelSequenceDecoder, uint32_t segIdx, uint32_t descIdx) {
    return to_c_handle(reinterpret_cast<ModelSequenceTableDecoder *>(modelSequenceDecoder)
                           ->getDescriptorBindingSlotsHandle(segIdx, descIdx));
}

mlsdk_decoder_binding_slots_handle mlsdk_decoder_model_sequence_get_segment_input_binding_slot(
    mlsdk_decoder_model_sequence_decoder *const modelSequenceDecoder, uint32_t segIdx) {
    assert(modelSequenceDecoder != nullptr && "modelSequenceDecoder is null");
    return to_c_handle(
        reinterpret_cast<ModelSequenceTableDecoder *>(modelSequenceDecoder)->getSegmentInputBindingSlotsHandle(segIdx));
}

mlsdk_decoder_binding_slots_handle mlsdk_decoder_model_sequence_get_segment_output_binding_slot(
    mlsdk_decoder_model_sequence_decoder *const modelSequenceDecoder, uint32_t segIdx) {
    assert(modelSequenceDecoder != nullptr && "modelSequenceDecoder is null");
    return to_c_handle(reinterpret_cast<ModelSequenceTableDecoder *>(modelSequenceDecoder)
                           ->getSegmentOutputBindingSlotsHandle(segIdx));
}

mlsdk_decoder_binding_slots_handle
mlsdk_decoder_model_sequence_get_input_binding_slot(mlsdk_decoder_model_sequence_decoder *const modelSequenceDecoder) {
    assert(modelSequenceDecoder != nullptr && "modelSequenceDecoder is null");
    return to_c_handle(
        reinterpret_cast<ModelSequenceTableDecoder *>(modelSequenceDecoder)->getModelSequenceInputBindingSlotsHandle());
}

mlsdk_decoder_binding_slots_handle
mlsdk_decoder_model_sequence_get_output_binding_slot(mlsdk_decoder_model_sequence_decoder *const modelSequenceDecoder) {
    assert(modelSequenceDecoder != nullptr && "modelSequenceDecoder is null");
    return to_c_handle(reinterpret_cast<ModelSequenceTableDecoder *>(modelSequenceDecoder)
                           ->getModelSequenceOutputBindingSlotsHandle());
}

size_t mlsdk_decoder_model_resource_table_decoder_mem_reqs() { return ModelResourceTableDecoderSize(); }

bool mlsdk_decoder_is_valid_model_resource_table(const void *modelResourceTableData, const uint64_t size) {
    assert(modelResourceTableData != nullptr && "modelResourceTableData is null");
    return VerifyModelResourceTable(modelResourceTableData, size);
}

mlsdk_decoder_model_resource_table_decoder *
mlsdk_decoder_create_model_resource_table_decoder(const void *const modelResourceTableData, void *decoderMemory) {
    assert(modelResourceTableData != nullptr && "modelResourceTableData is null");
    assert(decoderMemory != nullptr && "decoderMemory is null");
    return reinterpret_cast<mlsdk_decoder_model_resource_table_decoder *>(
        CreateModelResourceTableDecoderInPlace(modelResourceTableData, decoderMemory));
}

size_t mlsdk_decoder_get_model_resource_table_num_entries(
    const mlsdk_decoder_model_resource_table_decoder *const modelResourceTableDecoder) {
    assert(modelResourceTableDecoder != nullptr && "modelResourceTableDecoder is null");
    return reinterpret_cast<const ModelResourceTableDecoder *>(modelResourceTableDecoder)->size();
}

mlsdk_vk_descriptor_type_optional
mlsdk_decoder_get_vk_descriptor_type(const mlsdk_decoder_model_resource_table_decoder *const modelResourceTableDecoder,
                                     uint32_t idx) {
    assert(modelResourceTableDecoder != nullptr && "modelResourceTableDecoder is null");
    return convert_descriptor_type(
        reinterpret_cast<const ModelResourceTableDecoder *>(modelResourceTableDecoder)->getDescriptorType(idx));
}

mlsdk_vk_format mlsdk_vk_format_undefined() { return UndefinedFormat(); }

mlsdk_vk_format
mlsdk_decoder_get_vk_format(const mlsdk_decoder_model_resource_table_decoder *const modelResourceTableDecoder,
                            uint32_t idx) {
    assert(modelResourceTableDecoder != nullptr && "modelResourceTableDecoder is null");
    return convert_vk_format(
        reinterpret_cast<const ModelResourceTableDecoder *>(modelResourceTableDecoder)->getVkFormat(idx));
}

/**********************************************************************************************************************/

bool mlsdk_decoder_is_valid_constant_table(const void *constantTableData, const uint64_t size) {
    assert(constantTableData != nullptr && "constantTableData is null");
    return VerifyConstant(constantTableData, size);
}

mlsdk_decoder_constant_table_decoder *mlsdk_decoder_create_constant_table_decoder(const void *const constantTableData,
                                                                                  const uint64_t size,
                                                                                  void *constantDecoderMemory) {
    assert(constantTableData != nullptr && "constantTableData is null");
    assert(constantDecoderMemory != nullptr && "constantDecoderMemory is null");
    return reinterpret_cast<mlsdk_decoder_constant_table_decoder *>(
        CreateConstantDecoderInPlace(constantTableData, size, constantDecoderMemory));
}

size_t mlsdk_decoder_constant_table_decoder_mem_reqs() { return ConstantDecoderSize(); }

void mlsdk_decoder_constant_table_get_data(const mlsdk_decoder_constant_table_decoder *const constantDecoder,
                                           uint32_t idx, mlsdk_decoder_constant_data *constantData) {
    assert(constantDecoder != nullptr && "constantDecoder is null");
    assert(constantData != nullptr && "constantData is null");
    DataView<uint8_t> view = reinterpret_cast<const ConstantDecoder *>(constantDecoder)->getConstant(idx);
    constantData->data = view.begin();
    constantData->size = view.size();
}

uint32_t mlsdk_decoder_constant_table_get_mrt_index(const mlsdk_decoder_constant_table_decoder *const constantDecoder,
                                                    uint32_t constidx) {
    assert(constantDecoder != nullptr && "constantDecoder is null");
    return reinterpret_cast<const ConstantDecoder *>(constantDecoder)->getConstantMrtIndex(constidx);
}

bool mlsdk_decoder_constant_table_is_sparse(const mlsdk_decoder_constant_table_decoder *const constantDecoder,
                                            uint32_t constidx) {
    assert(constantDecoder != nullptr && "constantDecoder is null");
    return reinterpret_cast<const ConstantDecoder *>(constantDecoder)->isSparseConstant(constidx);
}

int64_t
mlsdk_decoder_constant_table_get_sparsity_dimension(const mlsdk_decoder_constant_table_decoder *const constantDecoder,
                                                    uint32_t constidx) {
    assert(constantDecoder != nullptr && "constantDecoder is null");
    return reinterpret_cast<const ConstantDecoder *>(constantDecoder)->getConstantSparsityDimension(constidx);
}

size_t mlsdk_decoder_get_constant_table_num_entries(const mlsdk_decoder_constant_table_decoder *const constantDecoder) {
    assert(constantDecoder != nullptr && "constantDecoder is null");
    return reinterpret_cast<const ConstantDecoder *>(constantDecoder)->size();
}

/**********************************************************************************************************************/
namespace {
inline mlsdk_decoder_mrt_category convert_resource_category(ResourceCategory category) {
    switch (category) {
    case ResourceCategory::INPUT:
        return mlsdk_decoder_mrt_category_input;
    case ResourceCategory::OUTPUT:
        return mlsdk_decoder_mrt_category_output;
    case ResourceCategory::INTERMEDIATE:
        return mlsdk_decoder_mrt_category_intermediate;
    case ResourceCategory::CONSTANT:
        return mlsdk_decoder_mrt_category_constant;
    default:
        assert(false && "unknown type");
        return mlsdk_decoder_mrt_category_constant;
    }
}
} // namespace

mlsdk_decoder_mrt_category mlsdk_decoder_model_resource_table_get_category(
    const mlsdk_decoder_model_resource_table_decoder *const modelResourceTableDecoder, uint32_t mrtIdx) {
    assert(modelResourceTableDecoder != nullptr && "modelResourceTableDecoder is null");
    return convert_resource_category(
        reinterpret_cast<const ModelResourceTableDecoder *>(modelResourceTableDecoder)->getCategory(mrtIdx));
}

void mlsdk_decoder_model_resource_table_get_tensor_shape(
    const mlsdk_decoder_model_resource_table_decoder *const modelResourceTableDecoder, uint32_t mrtIdx,
    mlsdk_decoder_tensor_dimensions *dimensions) {
    assert(modelResourceTableDecoder != nullptr && "modelResourceTableDecoder is null");
    auto tensorShape =
        reinterpret_cast<const ModelResourceTableDecoder *>(modelResourceTableDecoder)->getTensorShape(mrtIdx);
    dimensions->data = tensorShape.begin();
    dimensions->size = tensorShape.size();
}

void mlsdk_decoder_model_resource_table_get_tensor_strides(
    const mlsdk_decoder_model_resource_table_decoder *const modelResourceTableDecoder, uint32_t mrtIdx,
    mlsdk_decoder_tensor_dimensions *dimensions) {
    assert(modelResourceTableDecoder != nullptr && "modelResourceTableDecoder is null");
    auto tensorStrides =
        reinterpret_cast<const ModelResourceTableDecoder *>(modelResourceTableDecoder)->getTensorStride(mrtIdx);
    dimensions->data = tensorStrides.begin();
    dimensions->size = tensorStrides.size();
}
