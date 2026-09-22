/*
 * SPDX-FileCopyrightText: Copyright 2024-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "header.hpp"
#include "vgf/decoder.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using namespace mlsdk::vgflib;

namespace {

template <typename T> py::object pyDataView(DataView<T> data) {
    if (data.begin() && !data.empty()) {
        return py::memoryview::from_buffer(data.begin(), {data.size()}, {sizeof(T)});
    }
    return py::none();
};

} // namespace

// Header Decoder

class PyHeaderDecoder final : public HeaderDecoder {
  public:
    using HeaderDecoder::HeaderDecoder;

    bool IsLatestVersion() const override { PYBIND11_OVERRIDE_PURE(bool, HeaderDecoder, IsLatestVersion); }

    bool IsValid() const override { PYBIND11_OVERRIDE_PURE(bool, HeaderDecoder, IsValid); }

    uint16_t GetEncoderVulkanHeadersVersion() const override {
        PYBIND11_OVERRIDE_PURE(uint16_t, HeaderDecoder, GetEncoderVulkanHeadersVersion);
    }

    FormatVersion GetVersion() const override { PYBIND11_OVERRIDE_PURE(FormatVersion, HeaderDecoder, GetVersion); }

    bool CheckVersion() const override { PYBIND11_OVERRIDE_PURE(bool, HeaderDecoder, CheckVersion); }

    uint8_t GetMajor() const override { PYBIND11_OVERRIDE_PURE(uint8_t, HeaderDecoder, GetMajor); }

    uint8_t GetMinor() const override { PYBIND11_OVERRIDE_PURE(uint8_t, HeaderDecoder, GetMinor); }

    uint8_t GetPatch() const override { PYBIND11_OVERRIDE_PURE(uint8_t, HeaderDecoder, GetPatch); }

    uint64_t GetModuleTableSize() const override {
        PYBIND11_OVERRIDE_PURE(uint64_t, HeaderDecoder, GetModuleTableSize);
    }

    uint64_t GetModuleTableOffset() const override {
        PYBIND11_OVERRIDE_PURE(uint64_t, HeaderDecoder, GetModuleTableOffset);
    }

    uint64_t GetModelSequenceTableOffset() const override {
        PYBIND11_OVERRIDE_PURE(uint64_t, HeaderDecoder, GetModelSequenceTableOffset);
    }

    uint64_t GetModelSequenceTableSize() const override {
        PYBIND11_OVERRIDE_PURE(uint64_t, HeaderDecoder, GetModelSequenceTableSize);
    }

    uint64_t GetModelResourceTableOffset() const override {
        PYBIND11_OVERRIDE_PURE(uint64_t, HeaderDecoder, GetModelResourceTableOffset);
    }

    uint64_t GetModelResourceTableSize() const override {
        PYBIND11_OVERRIDE_PURE(uint64_t, HeaderDecoder, GetModelResourceTableSize);
    }

    uint64_t GetConstantsSize() const override { PYBIND11_OVERRIDE_PURE(uint64_t, HeaderDecoder, GetConstantsSize); }

    uint64_t GetConstantsOffset() const override {
        PYBIND11_OVERRIDE_PURE(uint64_t, HeaderDecoder, GetConstantsOffset);
    }
};

void pyInitHeaderDecoder(py::module_ &m) {

    py::class_<FormatVersion>(m, "FormatVersion", "Semantic version stored in a VGF header.")
        .def(py::init<>())
        .def_readwrite("major", &FormatVersion::major, "Major version.")
        .def_readwrite("minor", &FormatVersion::minor, "Minor version.")
        .def_readwrite("patch", &FormatVersion::patch, "Patch version.");

    py::class_<HeaderDecoder, PyHeaderDecoder>(m, "HeaderDecoder", "Inspect and validate a VGF header.")
        .def(py::init<>())
        .def("IsLatestVersion", &HeaderDecoder::IsLatestVersion, "Return whether the file uses the latest VGF version.")
        .def("IsValid", &HeaderDecoder::IsValid, "Return whether the VGF header magic value is valid.")
        .def("GetEncoderVulkanHeadersVersion", &HeaderDecoder::GetEncoderVulkanHeadersVersion,
             "Return the Vulkan header version used by the encoder.")
        .def("GetVersion", &HeaderDecoder::GetVersion, "Return the VGF format version.")
        .def("CheckVersion", &HeaderDecoder::CheckVersion, "Return whether the VGF format version is supported.")
        .def("GetMajor", &HeaderDecoder::GetMajor, "Return the VGF major version.")
        .def("GetMinor", &HeaderDecoder::GetMinor, "Return the VGF minor version.")
        .def("GetPatch", &HeaderDecoder::GetPatch, "Return the VGF patch version.")
        .def("GetModuleTableSize", &HeaderDecoder::GetModuleTableSize, "Return the module-table size in bytes.")
        .def("GetModuleTableOffset", &HeaderDecoder::GetModuleTableOffset, "Return the module-table byte offset.")
        .def("GetModelSequenceTableOffset", &HeaderDecoder::GetModelSequenceTableOffset,
             "Return the model-sequence-table byte offset.")
        .def("GetModelSequenceTableSize", &HeaderDecoder::GetModelSequenceTableSize,
             "Return the model-sequence-table size in bytes.")
        .def("GetModelResourceTableOffset", &HeaderDecoder::GetModelResourceTableOffset,
             "Return the model-resource-table byte offset.")
        .def("GetModelResourceTableSize", &HeaderDecoder::GetModelResourceTableSize,
             "Return the model-resource-table size in bytes.")
        .def("GetConstantsSize", &HeaderDecoder::GetConstantsSize, "Return the constant-section size in bytes.")
        .def("GetConstantsOffset", &HeaderDecoder::GetConstantsOffset, "Return the constant-section byte offset.");

    m.def("HeaderSize", &HeaderSize, "Return the encoded VGF header size in bytes.");
    m.def("HeaderDecoderSize", &HeaderDecoderSize, "Return the native header decoder size in bytes.");
    m.def(
        "CreateHeaderDecoder",
        [](const py::buffer &buffer, uint64_t headerSize, uint64_t fileSize) {
            return CreateHeaderDecoder(buffer.request().ptr, headerSize, fileSize);
        },
        "Create a header decoder over a buffer containing VGF file data.", py::keep_alive<0, 1>(), py::arg("data"),
        py::arg("headerSize"), py::arg("fileSize"));

    m.attr("HEADER_MAGIC_VALUE_OLD") = HEADER_MAGIC_VALUE_OLD;
    m.attr("HEADER_MAGIC_VALUE") = HEADER_MAGIC_VALUE;
    m.attr("HEADER_MAGIC_OFFSET") = HEADER_MAGIC_OFFSET;
    m.attr("HEADER_VK_HEADER_VERSION_OFFSET") = HEADER_VK_HEADER_VERSION_OFFSET;
    m.attr("HEADER_VERSION_OFFSET") = HEADER_VERSION_OFFSET;
    m.attr("HEADER_HEADER_SIZE_VALUE") = HEADER_HEADER_SIZE_VALUE;

    m.attr("HEADER_FIRST_SECTION_OFFSET") = HEADER_FIRST_SECTION_OFFSET;
    m.attr("HEADER_SECOND_SECTION_OFFSET") = HEADER_SECOND_SECTION_OFFSET;
    m.attr("HEADER_THIRD_SECTION_OFFSET") = HEADER_THIRD_SECTION_OFFSET;
    m.attr("HEADER_FOURTH_SECTION_OFFSET") = HEADER_FOURTH_SECTION_OFFSET;

    m.attr("HEADER_MODULE_SECTION_OFFSET") = HEADER_MODULE_SECTION_OFFSET;
    m.attr("HEADER_MODULE_SECTION_OFFSET_OFFSET") = HEADER_MODULE_SECTION_OFFSET_OFFSET;
    m.attr("HEADER_MODULE_SECTION_SIZE_OFFSET") = HEADER_MODULE_SECTION_SIZE_OFFSET;

    m.attr("HEADER_MODEL_SEQUENCE_SECTION_OFFSET") = HEADER_MODEL_SEQUENCE_SECTION_OFFSET;
    m.attr("HEADER_MODEL_SEQUENCE_SECTION_OFFSET_OFFSET") = HEADER_MODEL_SEQUENCE_SECTION_OFFSET_OFFSET;
    m.attr("HEADER_MODEL_SEQUENCE_SECTION_SIZE_OFFSET") = HEADER_MODEL_SEQUENCE_SECTION_SIZE_OFFSET;

    m.attr("HEADER_MODEL_RESOURCE_SECTION_OFFSET") = HEADER_MODEL_RESOURCE_SECTION_OFFSET;
    m.attr("HEADER_MODEL_RESOURCE_SECTION_OFFSET_OFFSET") = HEADER_MODEL_RESOURCE_SECTION_OFFSET_OFFSET;
    m.attr("HEADER_MODEL_RESOURCE_SECTION_SIZE_OFFSET") = HEADER_MODEL_RESOURCE_SECTION_SIZE_OFFSET;

    m.attr("HEADER_CONSTANT_SECTION_OFFSET") = HEADER_CONSTANT_SECTION_OFFSET;
    m.attr("HEADER_CONSTANT_SECTION_OFFSET_OFFSET") = HEADER_CONSTANT_SECTION_OFFSET_OFFSET;
    m.attr("HEADER_CONSTANT_SECTION_SIZE_OFFSET") = HEADER_CONSTANT_SECTION_SIZE_OFFSET;

    m.attr("HEADER_MAJOR_VERSION_VALUE") = HEADER_MAJOR_VERSION_VALUE;
    m.attr("HEADER_MINOR_VERSION_VALUE") = HEADER_MINOR_VERSION_VALUE;
    m.attr("HEADER_PATCH_VERSION_VALUE") = HEADER_PATCH_VERSION_VALUE;
}

// Module Table Decoder

class PyModuleTableDecoder final : public ModuleTableDecoder {
  public:
    using ModuleTableDecoder::ModuleTableDecoder;

    size_t size() const override { PYBIND11_OVERRIDE_PURE(size_t, ModuleTableDecoder, size); }

    ModuleType getModuleType(uint32_t idx) const override {
        PYBIND11_OVERRIDE_PURE(ModuleType, ModuleTableDecoder, getModuleType, idx);
    }

    bool hasSPIRV(uint32_t idx) const override { return isSPIRV(idx); }

    bool isSPIRV(uint32_t idx) const override { PYBIND11_OVERRIDE_PURE(bool, ModuleTableDecoder, isSPIRV, idx); }

    bool hasSPIRVCode(uint32_t idx) const override {
        PYBIND11_OVERRIDE_PURE(bool, ModuleTableDecoder, hasSPIRVCode, idx);
    }

    bool isGLSL(uint32_t idx) const override { PYBIND11_OVERRIDE_PURE(bool, ModuleTableDecoder, isGLSL, idx); }

    bool hasGLSLCode(uint32_t idx) const override {
        PYBIND11_OVERRIDE_PURE(bool, ModuleTableDecoder, hasGLSLCode, idx);
    }

    bool isHLSL(uint32_t idx) const override { PYBIND11_OVERRIDE_PURE(bool, ModuleTableDecoder, isHLSL, idx); }

    bool hasHLSLCode(uint32_t idx) const override {
        PYBIND11_OVERRIDE_PURE(bool, ModuleTableDecoder, hasHLSLCode, idx);
    }

    std::string_view getModuleName(uint32_t idx) const override {
        PYBIND11_OVERRIDE_PURE(std::string_view, ModuleTableDecoder, getModuleName, idx);
    }

    std::string_view getModuleEntryPoint(uint32_t idx) const override {
        PYBIND11_OVERRIDE_PURE(std::string_view, ModuleTableDecoder, getModuleEntryPoint, idx);
    }

    DataView<uint32_t> getModuleCode(uint32_t idx) const override { return getSPIRVModuleCode(idx); }

    DataView<uint32_t> getSPIRVModuleCode(uint32_t idx) const override {
        PYBIND11_OVERRIDE_PURE(DataView<uint32_t>, ModuleTableDecoder, getSPIRVModuleCode, idx);
    }

    std::string_view getGLSLModuleCode(uint32_t idx) const override {
        PYBIND11_OVERRIDE_PURE(std::string_view, ModuleTableDecoder, getGLSLModuleCode, idx);
    }

    std::string_view getHLSLModuleCode(uint32_t idx) const override {
        PYBIND11_OVERRIDE_PURE(std::string_view, ModuleTableDecoder, getHLSLModuleCode, idx);
    }
};

void pyInitModuleTableDecoder(py::module_ &m) {

    py::class_<ModuleTableDecoder, PyModuleTableDecoder>(m, "ModuleTableDecoder",
                                                         "Inspect modules in a VGF module table.")
        .def(py::init<>())
        .def("size", &ModuleTableDecoder::size, "Return the number of modules.")
        .def("getModuleType", &ModuleTableDecoder::getModuleType, "Return the module type.", py::arg("idx"))
        .def(
            "hasSPIRV",
            [](const ModuleTableDecoder &decoder, uint32_t idx) {
                py::module_ warnings = py::module_::import("warnings");
                py::object deprecationWarning = py::module_::import("builtins").attr("DeprecationWarning");
                warnings.attr("warn")("ModuleTableDecoder.hasSPIRV is deprecated; use isSPIRV()", deprecationWarning,
                                      2);
                return decoder.isSPIRV(idx);
            },
            "Deprecated: return whether the module contains SPIR-V. Use :meth:`isSPIRV`.", py::arg("idx"))
        .def("isSPIRV", &ModuleTableDecoder::isSPIRV, "Return whether the module uses SPIR-V.", py::arg("idx"))
        .def("hasSPIRVCode", &ModuleTableDecoder::hasSPIRVCode, "Return whether SPIR-V code is present.",
             py::arg("idx"))
        .def("isGLSL", &ModuleTableDecoder::isGLSL, "Return whether the module uses GLSL.", py::arg("idx"))
        .def("hasGLSLCode", &ModuleTableDecoder::hasGLSLCode, "Return whether GLSL code is present.", py::arg("idx"))
        .def("isHLSL", &ModuleTableDecoder::isHLSL, "Return whether the module uses HLSL.", py::arg("idx"))
        .def("hasHLSLCode", &ModuleTableDecoder::hasHLSLCode, "Return whether HLSL code is present.", py::arg("idx"))
        .def("getModuleName", &ModuleTableDecoder::getModuleName, "Return the module name.", py::arg("idx"))
        .def("getModuleEntryPoint", &ModuleTableDecoder::getModuleEntryPoint, "Return the module entry point.",
             py::arg("idx"))
        .def(
            "getModuleCode",
            [](const ModuleTableDecoder &decoder, uint32_t idx) {
                py::module_ warnings = py::module_::import("warnings");
                py::object deprecationWarning = py::module_::import("builtins").attr("DeprecationWarning");
                warnings.attr("warn")("ModuleTableDecoder.getModuleCode is deprecated; use getSPIRVModuleCode()",
                                      deprecationWarning, 2);
                return pyDataView<uint32_t>(decoder.getSPIRVModuleCode(idx));
            },
            "Deprecated: return SPIR-V code as a memoryview. Use :meth:`getSPIRVModuleCode`.", py::arg("idx"))
        .def(
            "getSPIRVModuleCode",
            [](const ModuleTableDecoder &decoder, uint32_t idx) {
                return pyDataView<uint32_t>(decoder.getSPIRVModuleCode(idx));
            },
            "Return SPIR-V code as a memoryview, or None when absent.", py::arg("idx"))
        .def("getGLSLModuleCode", &ModuleTableDecoder::getGLSLModuleCode, "Return GLSL source, or an empty string.",
             py::arg("idx"))
        .def("getHLSLModuleCode", &ModuleTableDecoder::getHLSLModuleCode, "Return HLSL source, or an empty string.",
             py::arg("idx"));

    m.def("ModuleTableDecoderSize", &ModuleTableDecoderSize, "Return the native module-table decoder size in bytes.");
    m.def(
        "CreateModuleTableDecoder",
        [](const py::buffer &buffer, uint64_t size) { return CreateModuleTableDecoder(buffer.request().ptr, size); },
        "Create a module-table decoder over an encoded section.", py::keep_alive<0, 1>(), py::arg("data"),
        py::arg("size"));
}

// Model Sequence Decoder

class PyModelSequenceTableDecoder final : public ModelSequenceTableDecoder {
  public:
    using ModelSequenceTableDecoder::ModelSequenceTableDecoder;

    size_t modelSequenceTableSize() const override {
        PYBIND11_OVERRIDE_PURE(size_t, ModelSequenceTableDecoder, modelSequenceTableSize);
    }

    size_t getSegmentDescriptorSetInfosSize(uint32_t segmentIdx) const override {
        PYBIND11_OVERRIDE_PURE(size_t, ModelSequenceTableDecoder, getSegmentDescriptorSetInfosSize, segmentIdx);
    }

    uint32_t getSegmentDescriptorSetIndex(uint32_t segmentIdx, uint32_t descIdx) const override {
        PYBIND11_OVERRIDE_PURE(uint32_t, ModelSequenceTableDecoder, getSegmentDescriptorSetIndex, segmentIdx, descIdx);
    }

    DataView<uint32_t> getSegmentConstantIndexes(uint32_t segmentIdx) const override {
        PYBIND11_OVERRIDE_PURE(DataView<uint32_t>, ModelSequenceTableDecoder, getSegmentConstantIndexes, segmentIdx);
    }

    GraphConstantBindingArrayHandle getSegmentConstantBindingsHandle(uint32_t segmentIdx) const override {
        PYBIND11_OVERRIDE_PURE(GraphConstantBindingArrayHandle, ModelSequenceTableDecoder,
                               getSegmentConstantBindingsHandle, segmentIdx);
    }

    size_t getGraphConstantBindingsSize(GraphConstantBindingArrayHandle handle) const override {
        PYBIND11_OVERRIDE_PURE(size_t, ModelSequenceTableDecoder, getGraphConstantBindingsSize, handle);
    }

    GraphConstantBinding getGraphConstantBinding(GraphConstantBindingArrayHandle handle,
                                                 uint32_t bindingIdx) const override {
        PYBIND11_OVERRIDE_PURE(GraphConstantBinding, ModelSequenceTableDecoder, getGraphConstantBinding, handle,
                               bindingIdx);
    }

    ModuleType getSegmentType(uint32_t segmentIdx) const override {
        PYBIND11_OVERRIDE_PURE(ModuleType, ModelSequenceTableDecoder, getSegmentType, segmentIdx);
    }

    std::string_view getSegmentName(uint32_t segmentIdx) const override {
        PYBIND11_OVERRIDE_PURE(std::string_view, ModelSequenceTableDecoder, getSegmentName, segmentIdx);
    }

    uint32_t getSegmentModuleIndex(uint32_t segmentIdx) const override {
        PYBIND11_OVERRIDE_PURE(uint32_t, ModelSequenceTableDecoder, getSegmentModuleIndex, segmentIdx);
    }

    DataView<uint32_t> getSegmentDispatchShape(uint32_t segmentIdx) const override {
        PYBIND11_OVERRIDE_PURE(DataView<uint32_t>, ModelSequenceTableDecoder, getSegmentDispatchShape, segmentIdx);
    }

    BindingSlotArrayHandle getDescriptorBindingSlotsHandle(uint32_t segmentIdx, uint32_t descIdx) const override {
        PYBIND11_OVERRIDE_PURE(BindingSlotArrayHandle, ModelSequenceTableDecoder, getDescriptorBindingSlotsHandle,
                               segmentIdx, descIdx);
    }

    BindingSlotArrayHandle getSegmentInputBindingSlotsHandle(uint32_t segmentIdx) const override {
        PYBIND11_OVERRIDE_PURE(BindingSlotArrayHandle, ModelSequenceTableDecoder, getSegmentInputBindingSlotsHandle,
                               segmentIdx);
    }

    BindingSlotArrayHandle getSegmentOutputBindingSlotsHandle(uint32_t segmentIdx) const override {
        PYBIND11_OVERRIDE_PURE(BindingSlotArrayHandle, ModelSequenceTableDecoder, getSegmentOutputBindingSlotsHandle,
                               segmentIdx);
    }

    BindingSlotArrayHandle getModelSequenceInputBindingSlotsHandle() const override {
        PYBIND11_OVERRIDE_PURE(BindingSlotArrayHandle, ModelSequenceTableDecoder,
                               getModelSequenceInputBindingSlotsHandle);
    }

    BindingSlotArrayHandle getModelSequenceOutputBindingSlotsHandle() const override {
        PYBIND11_OVERRIDE_PURE(BindingSlotArrayHandle, ModelSequenceTableDecoder,
                               getModelSequenceOutputBindingSlotsHandle);
    }

    NameArrayHandle getModelSequenceInputNamesHandle() const override {
        PYBIND11_OVERRIDE_PURE(NameArrayHandle, ModelSequenceTableDecoder, getModelSequenceInputNamesHandle);
    }

    NameArrayHandle getModelSequenceOutputNamesHandle() const override {
        PYBIND11_OVERRIDE_PURE(NameArrayHandle, ModelSequenceTableDecoder, getModelSequenceOutputNamesHandle);
    }

    size_t getNamesSize(NameArrayHandle handle) const override {
        PYBIND11_OVERRIDE_PURE(size_t, ModelSequenceTableDecoder, getNamesSize, handle);
    }

    std::string_view getName(NameArrayHandle handle, uint32_t nameIdx) const override {
        PYBIND11_OVERRIDE_PURE(std::string_view, ModelSequenceTableDecoder, getName, handle, nameIdx);
    }

    size_t getBindingsSize(BindingSlotArrayHandle handle) const override {
        PYBIND11_OVERRIDE_PURE(size_t, ModelSequenceTableDecoder, getBindingsSize, handle);
    }

    uint32_t getBindingSlotBinding(BindingSlotArrayHandle handle, uint32_t slotIdx) const override {
        PYBIND11_OVERRIDE_PURE(uint32_t, ModelSequenceTableDecoder, getBindingSlotBinding, handle, slotIdx);
    }

    uint32_t getBindingSlotMrtIndex(BindingSlotArrayHandle handle, uint32_t slotIdx) const override {
        PYBIND11_OVERRIDE_PURE(uint32_t, ModelSequenceTableDecoder, getBindingSlotMrtIndex, handle, slotIdx);
    }

    PushConstantRangeHandle getSegmentPushConstRange(uint32_t segmentIdx) const override {
        PYBIND11_OVERRIDE_PURE(PushConstantRangeHandle, ModelSequenceTableDecoder, getSegmentPushConstRange,
                               segmentIdx);
    }

    size_t getPushConstRangesSize(PushConstantRangeHandle handle) const override {
        PYBIND11_OVERRIDE_PURE(size_t, ModelSequenceTableDecoder, getPushConstRangesSize, handle);
    }

    uint32_t getPushConstRangeStageFlags(PushConstantRangeHandle handle, uint32_t rangeIdx) const override {
        PYBIND11_OVERRIDE_PURE(uint32_t, ModelSequenceTableDecoder, getPushConstRangeStageFlags, handle, rangeIdx);
    }

    uint32_t getPushConstRangeOffset(PushConstantRangeHandle handle, uint32_t rangeIdx) const override {
        PYBIND11_OVERRIDE_PURE(uint32_t, ModelSequenceTableDecoder, getPushConstRangeOffset, handle, rangeIdx);
    }

    uint32_t getPushConstRangeSize(PushConstantRangeHandle handle, uint32_t rangeIdx) const override {
        PYBIND11_OVERRIDE_PURE(uint32_t, ModelSequenceTableDecoder, getPushConstRangeSize, handle, rangeIdx);
    }
};

void pyInitModelSequenceTableDecoder(py::module_ &m) {

    py::class_<GraphConstantBinding>(m, "GraphConstantBinding",
                                     "Decoded mapping from a graph constant ID to a constant-table index.")
        .def(py::init<>())
        .def_readwrite("graphConstantId", &GraphConstantBinding::graphConstantId, "SPIR-V graph constant ID.")
        .def_readwrite("constantIndex", &GraphConstantBinding::constantIndex, "Constant-table index.");

    py::class_<ModelSequenceTableDecoder, PyModelSequenceTableDecoder>(
        m, "ModelSequenceTableDecoder", "Inspect segments and bindings in a VGF model-sequence table.")
        .def(py::init<>())
        .def("modelSequenceTableSize", &ModelSequenceTableDecoder::modelSequenceTableSize,
             "Return the number of model segments.")
        .def("getSegmentDescriptorSetInfosSize", &ModelSequenceTableDecoder::getSegmentDescriptorSetInfosSize,
             "Return the number of descriptor sets for a segment.", py::arg("segmentIdx"))
        .def("getSegmentDescriptorSetIndex", &ModelSequenceTableDecoder::getSegmentDescriptorSetIndex,
             "Return the explicit descriptor-set index.", py::arg("segmentIdx"), py::arg("descIdx"))
        .def(
            "getSegmentConstantIndexes",
            [](const ModelSequenceTableDecoder &decoder, uint32_t segmentIdx) {
                return pyDataView<uint32_t>(decoder.getSegmentConstantIndexes(segmentIdx));
            },
            "Return legacy constant-table indexes as a memoryview.", py::arg("segmentIdx"))
        .def(
            "getSegmentConstantBindings",
            [](const ModelSequenceTableDecoder &decoder, uint32_t segmentIdx) {
                const auto *const handle = decoder.getSegmentConstantBindingsHandle(segmentIdx);
                const auto size = decoder.getGraphConstantBindingsSize(handle);
                py::list result;
                for (uint32_t bindingIdx = 0; bindingIdx < size; ++bindingIdx) {
                    result.append(decoder.getGraphConstantBinding(handle, bindingIdx));
                }
                return result;
            },
            "Return graph constant bindings for a segment.", py::arg("segmentIdx"))
        .def("getSegmentType", &ModelSequenceTableDecoder::getSegmentType, "Return the segment module type.",
             py::arg("segmentIdx"))
        .def("getSegmentName", &ModelSequenceTableDecoder::getSegmentName, "Return the segment name.",
             py::arg("segmentIdx"))
        .def("getSegmentModuleIndex", &ModelSequenceTableDecoder::getSegmentModuleIndex,
             "Return the segment's module-table index.", py::arg("segmentIdx"))
        .def(
            "getSegmentDispatchShape",
            [](const ModelSequenceTableDecoder &decoder, uint32_t segmentIdx) {
                return pyDataView<uint32_t>(decoder.getSegmentDispatchShape(segmentIdx));
            },
            "Return the three-dimensional dispatch shape as a memoryview.", py::arg("segmentIdx"))
        .def("getDescriptorBindingSlotsHandle", &ModelSequenceTableDecoder::getDescriptorBindingSlotsHandle,
             "Return the binding slots for one descriptor set as an opaque handle.", py::return_value_policy::reference,
             py::arg("segmentIdx"), py::arg("descIdx"))
        .def("getSegmentInputBindingSlotsHandle", &ModelSequenceTableDecoder::getSegmentInputBindingSlotsHandle,
             "Return a segment's input binding slots as an opaque handle.", py::return_value_policy::reference,
             py::arg("segmentIdx"))
        .def("getSegmentOutputBindingSlotsHandle", &ModelSequenceTableDecoder::getSegmentOutputBindingSlotsHandle,
             "Return a segment's output binding slots as an opaque handle.", py::return_value_policy::reference,
             py::arg("segmentIdx"))
        .def("getModelSequenceInputBindingSlotsHandle",
             &ModelSequenceTableDecoder::getModelSequenceInputBindingSlotsHandle,
             "Return model input binding slots as an opaque handle.", py::return_value_policy::reference)
        .def("getModelSequenceOutputBindingSlotsHandle",
             &ModelSequenceTableDecoder::getModelSequenceOutputBindingSlotsHandle,
             "Return model output binding slots as an opaque handle.", py::return_value_policy::reference)
        .def("getModelSequenceInputNamesHandle", &ModelSequenceTableDecoder::getModelSequenceInputNamesHandle,
             "Return model input names as an opaque handle.", py::return_value_policy::reference)
        .def("getModelSequenceOutputNamesHandle", &ModelSequenceTableDecoder::getModelSequenceOutputNamesHandle,
             "Return model output names as an opaque handle.", py::return_value_policy::reference)
        .def("getNamesSize", &ModelSequenceTableDecoder::getNamesSize,
             "Return the number of names in an opaque handle.", py::arg("handle"))
        .def("getName", &ModelSequenceTableDecoder::getName, "Return a name from an opaque handle.", py::arg("handle"),
             py::arg("nameIdx"))
        .def("getBindingsSize", &ModelSequenceTableDecoder::getBindingsSize,
             "Return the number of binding slots in an opaque handle.", py::arg("handle"))
        .def("getBindingSlotBinding", &ModelSequenceTableDecoder::getBindingSlotBinding,
             "Return the Vulkan binding number for a binding slot.", py::arg("handle"), py::arg("slotIdx"))
        .def("getBindingSlotMrtIndex", &ModelSequenceTableDecoder::getBindingSlotMrtIndex,
             "Return the model-resource-table index for a binding slot.", py::arg("handle"), py::arg("slotIdx"))
        .def("getSegmentPushConstRange", &ModelSequenceTableDecoder::getSegmentPushConstRange,
             "Return a segment's push-constant ranges as an opaque handle.", py::arg("segmentIdx"),
             py::return_value_policy::reference)
        .def("getPushConstRangesSize", &ModelSequenceTableDecoder::getPushConstRangesSize,
             "Return the number of push-constant ranges in an opaque handle.", py::arg("handle"))
        .def("getPushConstRangeStageFlags", &ModelSequenceTableDecoder::getPushConstRangeStageFlags,
             "Return the Vulkan stage flags for a push-constant range.", py::arg("handle"), py::arg("rangeIdx"))
        .def("getPushConstRangeOffset", &ModelSequenceTableDecoder::getPushConstRangeOffset,
             "Return the byte offset of a push-constant range.", py::arg("handle"), py::arg("rangeIdx"))
        .def("getPushConstRangeSize", &ModelSequenceTableDecoder::getPushConstRangeSize,
             "Return the byte size of a push-constant range.", py::arg("handle"), py::arg("rangeIdx"));

    m.def("ModelSequenceTableDecoderSize", &ModelSequenceTableDecoderSize,
          "Return the native model-sequence-table decoder size in bytes.");
    m.def(
        "CreateModelSequenceTableDecoder",
        [](const py::buffer &buffer, uint64_t size) {
            return CreateModelSequenceTableDecoder(buffer.request().ptr, size);
        },
        "Create a model-sequence-table decoder over an encoded section.", py::keep_alive<0, 1>(), py::arg("data"),
        py::arg("size"));
}

// Model Resource Table Decoder

class PyModelResourceTableDecoder final : public ModelResourceTableDecoder {
  public:
    using ModelResourceTableDecoder::ModelResourceTableDecoder;

    size_t size() const override { PYBIND11_OVERRIDE_PURE(size_t, ModelResourceTableDecoder, size); }

    std::optional<DescriptorType> getDescriptorType(uint32_t id) const override {
        PYBIND11_OVERRIDE_PURE(std::optional<DescriptorType>, ModelResourceTableDecoder, getDescriptorType, id);
    }

    std::optional<AliasGroupId> getAliasGroupId(uint32_t id) const override {
        PYBIND11_OVERRIDE_PURE(std::optional<AliasGroupId>, ModelResourceTableDecoder, getAliasGroupId, id);
    }

    FormatType getVkFormat(uint32_t id) const override {
        PYBIND11_OVERRIDE_PURE(FormatType, ModelResourceTableDecoder, getVkFormat, id);
    }

    ResourceCategory getCategory(uint32_t id) const override {
        PYBIND11_OVERRIDE_PURE(ResourceCategory, ModelResourceTableDecoder, getCategory, id);
    }

    DataView<int64_t> getTensorShape(uint32_t id) const override {
        PYBIND11_OVERRIDE_PURE(DataView<int64_t>, ModelResourceTableDecoder, getTensorShape, id);
    }

    DataView<int64_t> getTensorStride(uint32_t id) const override {
        PYBIND11_OVERRIDE_PURE(DataView<int64_t>, ModelResourceTableDecoder, getTensorStride, id);
    }

    SamplerConfigHandle getSamplerConfigHandle(uint32_t id) const override {
        PYBIND11_OVERRIDE_PURE(SamplerConfigHandle, ModelResourceTableDecoder, getSamplerConfigHandle, id);
    }

    uint32_t getSamplerConfigMinFilter(SamplerConfigHandle handle) const override {
        PYBIND11_OVERRIDE_PURE(uint32_t, ModelResourceTableDecoder, getSamplerConfigMinFilter, handle);
    }

    uint32_t getSamplerConfigMagFilter(SamplerConfigHandle handle) const override {
        PYBIND11_OVERRIDE_PURE(uint32_t, ModelResourceTableDecoder, getSamplerConfigMagFilter, handle);
    }

    uint32_t getSamplerConfigAddressModeU(SamplerConfigHandle handle) const override {
        PYBIND11_OVERRIDE_PURE(uint32_t, ModelResourceTableDecoder, getSamplerConfigAddressModeU, handle);
    }

    uint32_t getSamplerConfigAddressModeV(SamplerConfigHandle handle) const override {
        PYBIND11_OVERRIDE_PURE(uint32_t, ModelResourceTableDecoder, getSamplerConfigAddressModeV, handle);
    }

    uint32_t getSamplerConfigBorderColor(SamplerConfigHandle handle) const override {
        PYBIND11_OVERRIDE_PURE(uint32_t, ModelResourceTableDecoder, getSamplerConfigBorderColor, handle);
    }
};

void pyInitModelResourceTableDecoder(py::module_ &m) {
    py::class_<ModelResourceTableDecoder, PyModelResourceTableDecoder>(
        m, "ModelResourceTableDecoder", "Inspect resources and tensor metadata in a VGF model-resource table.")
        .def(py::init<>())
        .def("size", &ModelResourceTableDecoder::size, "Return the number of model resources.")
        .def("getDescriptorType", &ModelResourceTableDecoder::getDescriptorType,
             "Return the Vulkan descriptor type, or None when absent.", py::arg("id"))
        .def("getAliasGroupId", &ModelResourceTableDecoder::getAliasGroupId,
             "Return the alias group ID, or None when absent.", py::arg("id"))
        .def("getVkFormat", &ModelResourceTableDecoder::getVkFormat, "Return the Vulkan format.", py::arg("id"))
        .def("getCategory", &ModelResourceTableDecoder::getCategory, "Return the resource usage category.",
             py::arg("id"))
        .def(
            "getTensorShape",
            [](const ModelResourceTableDecoder &decoder, uint32_t id) {
                return pyDataView<int64_t>(decoder.getTensorShape(id));
            },
            "Return the tensor shape as a memoryview, or None when absent.", py::arg("id"))
        .def(
            "getTensorStride",
            [](const ModelResourceTableDecoder &decoder, uint32_t id) {
                return pyDataView<int64_t>(decoder.getTensorStride(id));
            },
            "Return the tensor strides as a memoryview, or None when absent.", py::arg("id"))
        .def("getSamplerConfigHandle", &ModelResourceTableDecoder::getSamplerConfigHandle,
             "Return sampler configuration as an opaque handle, or None when absent.", py::arg("id"),
             py::return_value_policy::reference)
        .def("getSamplerConfigMinFilter", &ModelResourceTableDecoder::getSamplerConfigMinFilter,
             "Return the Vulkan minimum filter value.", py::arg("handle"))
        .def("getSamplerConfigMagFilter", &ModelResourceTableDecoder::getSamplerConfigMagFilter,
             "Return the Vulkan magnification filter value.", py::arg("handle"))
        .def("getSamplerConfigAddressModeU", &ModelResourceTableDecoder::getSamplerConfigAddressModeU,
             "Return the Vulkan U-axis address mode.", py::arg("handle"))
        .def("getSamplerConfigAddressModeV", &ModelResourceTableDecoder::getSamplerConfigAddressModeV,
             "Return the Vulkan V-axis address mode.", py::arg("handle"))
        .def("getSamplerConfigBorderColor", &ModelResourceTableDecoder::getSamplerConfigBorderColor,
             "Return the Vulkan border color value.", py::arg("handle"));

    m.def("ModelResourceTableDecoderSize", &ModelResourceTableDecoderSize,
          "Return the native model-resource-table decoder size in bytes.");
    m.def(
        "CreateModelResourceTableDecoder",
        [](const py::buffer &buffer, uint64_t size) {
            return CreateModelResourceTableDecoder(buffer.request().ptr, size);
        },
        "Create a model-resource-table decoder over an encoded section.", py::keep_alive<0, 1>(), py::arg("data"),
        py::arg("size"));
}

// Constant Decoder

class PyConstantDecoder final : public ConstantDecoder {
  public:
    using ConstantDecoder::ConstantDecoder;

    size_t size() const override { PYBIND11_OVERRIDE_PURE(size_t, ConstantDecoder, size); }

    uint32_t getConstantMrtIndex(uint32_t idx) const override {
        PYBIND11_OVERRIDE_PURE(uint32_t, ConstantDecoder, getConstantMrtIndex, idx);
    }

    bool isSparseConstant(uint32_t idx) const override {
        PYBIND11_OVERRIDE_PURE(bool, ConstantDecoder, isSparseConstant, idx);
    }

    int64_t getConstantSparsityDimension(uint32_t idx) const override {
        PYBIND11_OVERRIDE_PURE(int64_t, ConstantDecoder, getConstantSparsityDimension, idx);
    }

    DataView<uint8_t> getConstant(uint32_t idx) const override {
        PYBIND11_OVERRIDE_PURE(DataView<uint8_t>, ConstantDecoder, getConstant, idx);
    }
};

void pyInitConstantDecoder(py::module_ &m) {

    py::class_<ConstantDecoder, PyConstantDecoder>(m, "ConstantDecoder",
                                                   "Inspect constant metadata and data in a VGF constant section.")
        .def(py::init<>())
        .def("size", &ConstantDecoder::size, "Return the number of constants.")
        .def("getConstantMrtIndex", &ConstantDecoder::getConstantMrtIndex,
             "Return the model-resource-table index associated with a constant.", py::arg("idx"))
        .def("isSparseConstant", &ConstantDecoder::isSparseConstant, "Return whether a constant is sparse.",
             py::arg("idx"))
        .def("getConstantSparsityDimension", &ConstantDecoder::getConstantSparsityDimension,
             "Return the sparse dimension, or -1 for a non-sparse constant.", py::arg("idx"))
        .def(
            "getConstant",
            [&](const ConstantDecoder &decoder, uint32_t idx) { return pyDataView<uint8_t>(decoder.getConstant(idx)); },
            "Return constant bytes as a memoryview, or None when absent.", py::arg("idx"));

    m.def("ConstantDecoderSize", &ConstantDecoderSize, "Return the native constant decoder size in bytes.");
    m.def(
        "CreateConstantDecoder",
        [](const py::buffer &buffer, uint64_t size) { return CreateConstantDecoder(buffer.request().ptr, size); },
        "Create a constant decoder over an encoded section.", py::keep_alive<0, 1>(), py::arg("data"), py::arg("size"));
}

// Python Binding Module Decoder Setup

void pyInitDecoder(py::module_ &m) {

    py::class_<BindingSlotArrayHandle_s>(m, "BindingSlotArrayHandle_s", "Opaque binding-slot array handle.")
        .def(py::init<>());
    py::class_<NameArrayHandle_s>(m, "NameArrayHandle_s", "Opaque name array handle.").def(py::init<>());
    py::class_<PushConstantRangeHandle_s>(m, "PushConstantRangeHandle_s", "Opaque push-constant-range array handle.")
        .def(py::init<>());
    py::class_<SamplerConfigHandle_s>(m, "SamplerConfigHandle_s", "Opaque sampler configuration handle.");

    pyInitHeaderDecoder(m);
    pyInitModuleTableDecoder(m);
    pyInitModelSequenceTableDecoder(m);
    pyInitModelResourceTableDecoder(m);
    pyInitConstantDecoder(m);
}
