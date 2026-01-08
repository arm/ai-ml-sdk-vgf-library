/*
 * SPDX-FileCopyrightText: Copyright 2024-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf/encoder.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <sstream>

namespace py = pybind11;

using namespace mlsdk::vgflib;

class PyEncoder final : public Encoder {
  public:
    using Encoder::Encoder;

    ModuleRef AddModule(ModuleType type, const std::string &name, const std::string &entryPoint,
                        const std::vector<uint32_t> &code) override {
        PYBIND11_OVERRIDE_PURE(ModuleRef, Encoder, AddModule, type, name, entryPoint, code);
    }

    ModuleRef AddPlaceholderModule(ModuleType type, const std::string &name, const std::string &entryPoint) override {
        PYBIND11_OVERRIDE_PURE(ModuleRef, Encoder, AddPlaceholderModule, type, name, entryPoint);
    }

    BindingSlotRef AddBindingSlot(uint32_t binding, ResourceRef resource) override {
        PYBIND11_OVERRIDE_PURE(BindingSlotRef, Encoder, AddBindingSlot, binding, resource);
    }

    DescriptorSetInfoRef AddDescriptorSetInfo(const std::vector<BindingSlotRef> &bindings) override {
        PYBIND11_OVERRIDE_PURE(DescriptorSetInfoRef, Encoder, AddDescriptorSetInfo, bindings);
    }

    PushConstRangeRef AddPushConstRange(uint32_t stageFlags, uint32_t offset, uint32_t size) override {
        PYBIND11_OVERRIDE_PURE(PushConstRangeRef, Encoder, AddPushConstRange, stageFlags, offset, size);
    }

    SegmentInfoRef AddSegmentInfo(ModuleRef module, const std::string &name,
                                  const std::vector<DescriptorSetInfoRef> &descriptors,
                                  const std::vector<BindingSlotRef> &inputs, const std::vector<BindingSlotRef> &outputs,
                                  const std::vector<ConstantRef> &constants,
                                  const std::array<uint32_t, 3> &dispatchShape,
                                  const std::vector<PushConstRangeRef> &pushConstRanges) override {

        PYBIND11_OVERRIDE_PURE(SegmentInfoRef, Encoder, AddSegmentInfo, module, name, descriptors, inputs, outputs,
                               constants, dispatchShape, pushConstRanges);
    }

    void AddModelSequenceInputsOutputs(const std::vector<BindingSlotRef> &inputs,
                                       const std::vector<std::string> &inputNames,
                                       const std::vector<BindingSlotRef> &outputs,
                                       const std::vector<std::string> &outputNames) override {

        PYBIND11_OVERRIDE_PURE(void, Encoder, AddModelSequenceInputsOutputs, inputs, inputNames, outputs, outputNames);
    }

    ResourceRef AddInputResource(DescriptorType vkDescriptorType, FormatType vkFormat,
                                 const std::vector<int64_t> &shape, const std::vector<int64_t> &strides) override {
        PYBIND11_OVERRIDE_PURE(ResourceRef, Encoder, AddInputResource, vkDescriptorType, vkFormat, shape, strides);
    }

    ResourceRef AddOutputResource(DescriptorType vkDescriptorType, FormatType vkFormat,
                                  const std::vector<int64_t> &shape, const std::vector<int64_t> &strides) override {
        PYBIND11_OVERRIDE_PURE(ResourceRef, Encoder, AddOutputResource, vkDescriptorType, vkFormat, shape, strides);
    }

    ResourceRef AddIntermediateResource(DescriptorType vkDescriptorType, FormatType vkFormat,
                                        const std::vector<int64_t> &shape,
                                        const std::vector<int64_t> &strides) override {
        PYBIND11_OVERRIDE_PURE(ResourceRef, Encoder, AddIntermediateResource, vkDescriptorType, vkFormat, shape,
                               strides);
    }

    ResourceRef AddConstantResource(FormatType vkFormat, const std::vector<int64_t> &shape,
                                    const std::vector<int64_t> &strides) override {
        PYBIND11_OVERRIDE_PURE(ResourceRef, Encoder, AddConstantResource, vkFormat, shape, strides);
    }

    ConstantRef AddConstant(ResourceRef resourceRef, const void *data, size_t sizeInBytes,
                            int64_t sparsityDimension) override {
        PYBIND11_OVERRIDE_PURE(ConstantRef, Encoder, AddConstant, resourceRef, data, sizeInBytes, sparsityDimension);
    }

    void Finish() override { PYBIND11_OVERRIDE_PURE(void, Encoder, Finish); }

    bool WriteTo(std::ostream &output) override { PYBIND11_OVERRIDE_PURE(bool, Encoder, WriteTo, output); }
};

void pyInitEncoder(py::module m) {

    py::class_<ModuleRef>(m, "ModuleRef").def(py::init<uint32_t>()).def_readonly("reference", &ModuleRef::reference);
    py::class_<ResourceRef>(m, "ResourceRef")
        .def(py::init<uint32_t>())
        .def_readonly("reference", &ResourceRef::reference);
    py::class_<ConstantRef>(m, "ConstantRef")
        .def(py::init<uint32_t>())
        .def_readonly("reference", &ConstantRef::reference);
    py::class_<BindingSlotRef>(m, "BindingSlotRef")
        .def(py::init<uint32_t>())
        .def_readonly("reference", &BindingSlotRef::reference);
    py::class_<DescriptorSetInfoRef>(m, "DescriptorSetInfoRef")
        .def(py::init<uint32_t>())
        .def_readonly("reference", &DescriptorSetInfoRef::reference);
    py::class_<SegmentInfoRef>(m, "SegmentInfoRef")
        .def(py::init<uint32_t>())
        .def_readonly("reference", &SegmentInfoRef::reference);
    py::class_<PushConstRangeRef>(m, "PushConstRangeRef")
        .def(py::init<uint32_t>())
        .def_readonly("reference", &PushConstRangeRef::reference);

    py::class_<Encoder, PyEncoder>(m, "Encoder")
        .def(py::init<>())
        .def("AddModule", &Encoder::AddModule, py::arg("type"), py::arg("name"), py::arg("entryPoint"),
             py::arg("code") = py::list())
        .def("AddPlaceholderModule", &Encoder::AddPlaceholderModule, py::arg("type"), py::arg("name"),
             py::arg("entryPoint"))
        .def("AddBindingSlot", &Encoder::AddBindingSlot, py::arg("binding"), py::arg("resource"))
        .def("AddDescriptorSetInfo", &Encoder::AddDescriptorSetInfo, py::arg("bindings") = py::list())
        .def("AddPushConstRange", &Encoder::AddPushConstRange, py::arg("stageFlags"), py::arg("offset"),
             py::arg("size"))
        .def("AddSegmentInfo", &Encoder::AddSegmentInfo, py::arg("module"), py::arg("name"),
             py::arg("descriptors") = py::list(), py::arg("inputs") = py::list(), py::arg("outputs") = py::list(),
             py::arg("constants") = py::list(), py::arg("dispatchShape") = std::array<uint32_t, 3>(),
             py::arg("pushConstRanges") = py::list())
        .def("AddModelSequenceInputsOutputs", &Encoder::AddModelSequenceInputsOutputs, py::arg("inputs") = py::list(),
             py::arg("inputNames") = py::list(), py::arg("outputs") = py::list(), py::arg("outputNames") = py::list())
        .def("AddInputResource", &Encoder::AddInputResource, py::arg("vkDescriptorType"), py::arg("vkFormat"),
             py::arg("shape"), py::arg("strides"))
        .def("AddOutputResource", &Encoder::AddOutputResource, py::arg("vkDescriptorType"), py::arg("vkFormat"),
             py::arg("shape"), py::arg("strides"))
        .def("AddIntermediateResource", &Encoder::AddIntermediateResource, py::arg("vkDescriptorType"),
             py::arg("vkFormat"), py::arg("shape"), py::arg("strides"))
        .def("AddConstantResource", &Encoder::AddConstantResource, py::arg("vkFormat"), py::arg("shape"),
             py::arg("strides"))
        .def(
            "AddConstant",
            [](Encoder &encoder, ResourceRef resRef, const py::buffer &buffer, int64_t sparsityDimension) {
                return encoder.AddConstant(resRef, buffer.request().ptr,
                                           size_t(buffer.request().itemsize) * size_t(buffer.request().size),
                                           sparsityDimension);
            },
            py::arg("resourceRef"), py::arg("buffer"), py::arg("sparsityDimension") = CONSTANT_NOT_SPARSE_DIMENSION)
        .def("Finish", &Encoder::Finish)
        .def(
            "WriteTo",
            [](Encoder &encoder, py::object &pyIOStream) {
                if (!py::isinstance(pyIOStream, py::module::import("io").attr("IOBase"))) {
                    throw std::runtime_error("Object is not an IO stream");
                }

                std::stringstream stream;
                if (encoder.WriteTo(stream)) {
                    pyIOStream.attr("write")(py::bytes(stream.str()));
                    return true;
                }

                return false;
            },
            py::arg("output"));

    m.def("CreateEncoder", &CreateEncoder, py::arg("vkHeaderVersion"));
}
