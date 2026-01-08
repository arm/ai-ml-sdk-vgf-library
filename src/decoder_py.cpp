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
    if (data.begin() && data.size()) {
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

void pyInitHeaderDecoder(py::module m) {

    py::class_<HeaderDecoder, PyHeaderDecoder>(m, "HeaderDecoder")
        .def(py::init<>())
        .def("IsLatestVersion", &HeaderDecoder::IsLatestVersion)
        .def("IsValid", &HeaderDecoder::IsValid)
        .def("GetEncoderVulkanHeadersVersion", &HeaderDecoder::GetEncoderVulkanHeadersVersion)
        .def("CheckVersion", &HeaderDecoder::CheckVersion)
        .def("GetMajor", &HeaderDecoder::GetMajor)
        .def("GetMinor", &HeaderDecoder::GetMinor)
        .def("GetPatch", &HeaderDecoder::GetPatch)
        .def("GetModuleTableSize", &HeaderDecoder::GetModuleTableSize)
        .def("GetModuleTableOffset", &HeaderDecoder::GetModuleTableOffset)
        .def("GetModelSequenceTableOffset", &HeaderDecoder::GetModelSequenceTableOffset)
        .def("GetModelSequenceTableSize", &HeaderDecoder::GetModelSequenceTableSize)
        .def("GetModelResourceTableOffset", &HeaderDecoder::GetModelResourceTableOffset)
        .def("GetModelResourceTableSize", &HeaderDecoder::GetModelResourceTableSize)
        .def("GetConstantsSize", &HeaderDecoder::GetConstantsSize)
        .def("GetConstantsOffset", &HeaderDecoder::GetConstantsOffset);

    m.def("HeaderSize", &HeaderSize);
    m.def("HeaderDecoderSize", &HeaderDecoderSize);
    m.def(
        "CreateHeaderDecoder", [](const py::buffer &buffer) { return CreateHeaderDecoder(buffer.request().ptr); },
        py::arg("data"));

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

    bool hasSPIRV(uint32_t idx) const override { PYBIND11_OVERRIDE_PURE(bool, ModuleTableDecoder, hasSPIRV, idx); }

    std::string_view getModuleName(uint32_t idx) const override {
        PYBIND11_OVERRIDE_PURE(std::string_view, ModuleTableDecoder, getModuleName, idx);
    }

    std::string_view getModuleEntryPoint(uint32_t idx) const override {
        PYBIND11_OVERRIDE_PURE(std::string_view, ModuleTableDecoder, getModuleEntryPoint, idx);
    }

    DataView<uint32_t> getModuleCode(uint32_t idx) const override {
        PYBIND11_OVERRIDE_PURE(DataView<uint32_t>, ModuleTableDecoder, getModuleCode, idx);
    }
};

void pyInitModuleTableDecoder(py::module m) {

    py::class_<ModuleTableDecoder, PyModuleTableDecoder>(m, "ModuleTableDecoder")
        .def(py::init<>())
        .def("size", &ModuleTableDecoder::size)
        .def("getModuleType", &ModuleTableDecoder::getModuleType, py::arg("idx"))
        .def("hasSPIRV", &ModuleTableDecoder::hasSPIRV, py::arg("idx"))
        .def("getModuleName", &ModuleTableDecoder::getModuleName, py::arg("idx"))
        .def("getModuleEntryPoint", &ModuleTableDecoder::getModuleEntryPoint, py::arg("idx"))
        .def(
            "getModuleCode",
            [](const ModuleTableDecoder &decoder, uint32_t idx) {
                return pyDataView<uint32_t>(decoder.getModuleCode(idx));
            },
            py::arg("idx"));

    m.def("ModuleTableDecoderSize", &ModuleTableDecoderSize);
    m.def(
        "VerifyModuleTable",
        [](const py::buffer &buffer, uint64_t size) { return VerifyModuleTable(buffer.request().ptr, size); },
        py::arg("data"), py::arg("size"));
    m.def(
        "CreateModuleTableDecoder",
        [](const py::buffer &buffer) { return CreateModuleTableDecoder(buffer.request().ptr); }, py::arg("data"));
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

    DataView<uint32_t> getSegmentConstantIndexes(uint32_t segmentIdx) const override {
        PYBIND11_OVERRIDE_PURE(DataView<uint32_t>, ModelSequenceTableDecoder, getSegmentConstantIndexes, segmentIdx);
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

void pyInitModelSequenceTableDecoder(py::module m) {

    py::class_<ModelSequenceTableDecoder, PyModelSequenceTableDecoder>(m, "ModelSequenceTableDecoder")
        .def(py::init<>())
        .def("modelSequenceTableSize", &ModelSequenceTableDecoder::modelSequenceTableSize)
        .def("getSegmentDescriptorSetInfosSize", &ModelSequenceTableDecoder::getSegmentDescriptorSetInfosSize,
             py::arg("segmentIdx"))
        .def(
            "getSegmentConstantIndexes",
            [](const ModelSequenceTableDecoder &decoder, uint32_t segmentIdx) {
                return pyDataView<uint32_t>(decoder.getSegmentConstantIndexes(segmentIdx));
            },
            py::arg("segmentIdx"))
        .def("getSegmentType", &ModelSequenceTableDecoder::getSegmentType, py::arg("segmentIdx"))
        .def("getSegmentName", &ModelSequenceTableDecoder::getSegmentName, py::arg("segmentIdx"))
        .def("getSegmentModuleIndex", &ModelSequenceTableDecoder::getSegmentModuleIndex, py::arg("segmentIdx"))
        .def(
            "getSegmentDispatchShape",
            [](const ModelSequenceTableDecoder &decoder, uint32_t segmentIdx) {
                return pyDataView<uint32_t>(decoder.getSegmentDispatchShape(segmentIdx));
            },
            py::arg("segmentIdx"))
        .def("getDescriptorBindingSlotsHandle", &ModelSequenceTableDecoder::getDescriptorBindingSlotsHandle,
             py::return_value_policy::reference, py::arg("segmentIdx"), py::arg("descIdx"))
        .def("getSegmentInputBindingSlotsHandle", &ModelSequenceTableDecoder::getSegmentInputBindingSlotsHandle,
             py::return_value_policy::reference, py::arg("segmentIdx"))
        .def("getSegmentOutputBindingSlotsHandle", &ModelSequenceTableDecoder::getSegmentOutputBindingSlotsHandle,
             py::return_value_policy::reference, py::arg("segmentIdx"))
        .def("getModelSequenceInputBindingSlotsHandle",
             &ModelSequenceTableDecoder::getModelSequenceInputBindingSlotsHandle, py::return_value_policy::reference)
        .def("getModelSequenceOutputBindingSlotsHandle",
             &ModelSequenceTableDecoder::getModelSequenceOutputBindingSlotsHandle, py::return_value_policy::reference)
        .def("getModelSequenceInputNamesHandle", &ModelSequenceTableDecoder::getModelSequenceInputNamesHandle,
             py::return_value_policy::reference)
        .def("getModelSequenceOutputNamesHandle", &ModelSequenceTableDecoder::getModelSequenceOutputNamesHandle,
             py::return_value_policy::reference)
        .def("getNamesSize", &ModelSequenceTableDecoder::getNamesSize, py::arg("handle"))
        .def("getName", &ModelSequenceTableDecoder::getName, py::arg("handle"), py::arg("nameIdx"))
        .def("getBindingsSize", &ModelSequenceTableDecoder::getBindingsSize, py::arg("handle"))
        .def("getBindingSlotBinding", &ModelSequenceTableDecoder::getBindingSlotBinding, py::arg("handle"),
             py::arg("slotIdx"))
        .def("getBindingSlotMrtIndex", &ModelSequenceTableDecoder::getBindingSlotMrtIndex, py::arg("handle"),
             py::arg("slotIdx"))
        .def("getSegmentPushConstRange", &ModelSequenceTableDecoder::getSegmentPushConstRange, py::arg("segmentIdx"),
             py::return_value_policy::reference)
        .def("getPushConstRangesSize", &ModelSequenceTableDecoder::getPushConstRangesSize, py::arg("handle"))
        .def("getPushConstRangeStageFlags", &ModelSequenceTableDecoder::getPushConstRangeStageFlags, py::arg("handle"),
             py::arg("rangeIdx"))
        .def("getPushConstRangeOffset", &ModelSequenceTableDecoder::getPushConstRangeOffset, py::arg("handle"),
             py::arg("rangeIdx"))
        .def("getPushConstRangeSize", &ModelSequenceTableDecoder::getPushConstRangeSize, py::arg("handle"),
             py::arg("rangeIdx"));

    m.def(
        "VerifyModelSequenceTable",
        [](const py::buffer &buffer, uint64_t size) { return VerifyModelSequenceTable(buffer.request().ptr, size); },
        py::arg("data"), py::arg("size"));
    m.def(
        "CreateModelSequenceTableDecoder",
        [](const py::buffer &buffer) { return CreateModelSequenceTableDecoder(buffer.request().ptr); },
        py::arg("data"));
}

// Model Resource Table Decoder

class PyModelResourceTableDecoder final : public ModelResourceTableDecoder {
  public:
    using ModelResourceTableDecoder::ModelResourceTableDecoder;

    size_t size() const override { PYBIND11_OVERRIDE_PURE(size_t, ModelResourceTableDecoder, size); }

    std::optional<DescriptorType> getDescriptorType(uint32_t id) const override {
        PYBIND11_OVERRIDE_PURE(std::optional<DescriptorType>, ModelResourceTableDecoder, getDescriptorType, id);
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
};

void pyInitModelResourceTableDecoder(py::module m) {
    py::class_<ModelResourceTableDecoder, PyModelResourceTableDecoder>(m, "ModelResourceTableDecoder")
        .def(py::init<>())
        .def("size", &ModelResourceTableDecoder::size)
        .def("getDescriptorType", &ModelResourceTableDecoder::getDescriptorType, py::arg("id"))
        .def("getVkFormat", &ModelResourceTableDecoder::getVkFormat, py::arg("id"))
        .def("getCategory", &ModelResourceTableDecoder::getCategory, py::arg("id"))
        .def(
            "getTensorShape",
            [](const ModelResourceTableDecoder &decoder, uint32_t id) {
                return pyDataView<int64_t>(decoder.getTensorShape(id));
            },
            py::arg("id"))
        .def(
            "getTensorStride",
            [](const ModelResourceTableDecoder &decoder, uint32_t id) {
                return pyDataView<int64_t>(decoder.getTensorStride(id));
            },
            py::arg("id"));

    m.def("ModelResourceTableDecoderSize", &ModelResourceTableDecoderSize);
    m.def(
        "VerifyModelResourceTable",
        [](const py::buffer &buffer, uint64_t size) { return VerifyModelResourceTable(buffer.request().ptr, size); },
        py::arg("data"), py::arg("size"));
    m.def(
        "CreateModelResourceTableDecoder",
        [](const py::buffer &buffer) { return CreateModelResourceTableDecoder(buffer.request().ptr); },
        py::arg("data"));
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

void pyInitConstantDecoder(py::module m) {

    py::class_<ConstantDecoder, PyConstantDecoder>(m, "ConstantDecoder")
        .def(py::init<>())
        .def("size", &ConstantDecoder::size)
        .def("getConstantMrtIndex", &ConstantDecoder::getConstantMrtIndex, py::arg("idx"))
        .def("isSparseConstant", &ConstantDecoder::isSparseConstant, py::arg("idx"))
        .def("getConstantSparsityDimension", &ConstantDecoder::getConstantSparsityDimension, py::arg("idx"))
        .def(
            "getConstant",
            [&](const ConstantDecoder &decoder, uint32_t idx) { return pyDataView<uint8_t>(decoder.getConstant(idx)); },
            py::arg("idx"));

    m.def("ConstantDecoderSize", &ConstantDecoderSize);
    m.def(
        "VerifyConstant",
        [](const py::buffer &buffer, uint64_t size) { return VerifyConstant(buffer.request().ptr, size); },
        py::arg("data"), py::arg("size"));
    m.def(
        "CreateConstantDecoder",
        [](const py::buffer &buffer, uint64_t size) { return CreateConstantDecoder(buffer.request().ptr, size); },
        py::arg("data"), py::arg("size"));
}

// Python Binding Module Decoder Setup

void pyInitDecoder(py::module m) {

    py::class_<BindingSlotArrayHandle_s>(m, "BindingSlotArrayHandle_s").def(py::init<>());
    py::class_<NameArrayHandle_s>(m, "NameArrayHandle_s").def(py::init<>());
    py::class_<PushConstantRangeHandle_s>(m, "PushConstantRangeHandle_s").def(py::init<>());

    pyInitHeaderDecoder(m);
    pyInitModuleTableDecoder(m);
    pyInitModelSequenceTableDecoder(m);
    pyInitModelResourceTableDecoder(m);
    pyInitConstantDecoder(m);
}
