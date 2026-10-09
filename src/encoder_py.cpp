/*
 * SPDX-FileCopyrightText: Copyright 2024-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf/encoder.hpp"

#include <algorithm>
#include <limits>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <sstream>

namespace py = pybind11;

using namespace mlsdk::vgflib;

void pyInitEncoder(py::module_ &m) {

    py::class_<ModuleRef>(m, "ModuleRef", "Reference to an encoded module.")
        .def(py::init<uint32_t>())
        .def_readonly("reference", &ModuleRef::reference, "Module-table index.");
    py::class_<ResourceRef>(m, "ResourceRef", "Reference to an encoded model resource.")
        .def(py::init<uint32_t>())
        .def_readonly("reference", &ResourceRef::reference, "Model-resource-table index.");
    py::class_<ConstantRef>(m, "ConstantRef", "Reference to encoded constant data.")
        .def(py::init<uint32_t>())
        .def_readonly("reference", &ConstantRef::reference, "Constant-table index.");
    py::class_<GraphConstantBindingRef>(m, "GraphConstantBindingRef",
                                        "Map a graph constant ID to encoded constant data.")
        .def(py::init<uint32_t, ConstantRef>())
        .def_readwrite("graphConstantId", &GraphConstantBindingRef::graphConstantId, "SPIR-V graph constant ID.")
        .def_readonly("constant", &GraphConstantBindingRef::constant, "Encoded constant reference.");
    py::class_<BindingSlotRef>(m, "BindingSlotRef", "Reference to an encoded binding slot.")
        .def(py::init<uint32_t>())
        .def_readonly("reference", &BindingSlotRef::reference, "Binding-slot index.");
    py::class_<DescriptorSetInfoRef>(m, "DescriptorSetInfoRef", "Reference to encoded descriptor-set information.")
        .def(py::init<uint32_t>())
        .def_readonly("reference", &DescriptorSetInfoRef::reference, "Descriptor-set information index.");
    py::class_<SegmentInfoRef>(m, "SegmentInfoRef", "Reference to an encoded model segment.")
        .def(py::init<uint32_t>())
        .def_readonly("reference", &SegmentInfoRef::reference, "Model-segment index.");
    py::class_<PushConstRangeRef>(m, "PushConstRangeRef", "Reference to an encoded push-constant range.")
        .def(py::init<uint32_t>())
        .def_readonly("reference", &PushConstRangeRef::reference, "Push-constant-range index.");

    py::class_<Encoder>(m, "Encoder", py::is_final(), "Construct and serialize a VGF file.")
        .def("AddModule",
             py::overload_cast<ModuleType, const std::string &, const std::string &, const std::vector<uint32_t> &>(
                 &Encoder::AddModule),
             "Add a SPIR-V module and return its reference.", py::arg("type"), py::arg("name"), py::arg("entryPoint"),
             py::arg("code") = py::list())
        .def("AddModule",
             py::overload_cast<ModuleType, const std::string &, const std::string &, ShaderType, const std::string &>(
                 &Encoder::AddModule),
             "Add a source-language shader module and return its reference.", py::arg("type"), py::arg("name"),
             py::arg("entryPoint"), py::arg("shaderType"), py::arg("code") = "")
        .def(
            "AddPlaceholderModule",
            [](Encoder &encoder, ModuleType type, const std::string &name, const std::string &entryPoint) {
                py::module_ warnings = py::module_::import("warnings");
                py::object deprecationWarning = py::module_::import("builtins").attr("DeprecationWarning");
                warnings.attr("warn")("Encoder.AddPlaceholderModule is deprecated; use AddModule()",
                                      deprecationWarning);
                return encoder.AddModule(type, name, entryPoint);
            },
            "Deprecated: add a module without code. Use :meth:`AddModule`.", py::arg("type"), py::arg("name"),
            py::arg("entryPoint"))
        .def("AddBindingSlot", &Encoder::AddBindingSlot, "Add a binding slot associated with a model resource.",
             py::arg("binding"), py::arg("resource"))
        .def("AddDescriptorSetInfo", &Encoder::AddDescriptorSetInfo,
             "Add descriptor-set information for binding slots.", py::arg("bindings") = py::list(),
             py::arg("setIndex") = std::numeric_limits<uint32_t>::max())
        .def("AddPushConstRange", &Encoder::AddPushConstRange, "Add a push-constant range to a segment.",
             py::arg("stageFlags"), py::arg("offset"), py::arg("size"))
        .def(
            "AddSegmentInfo",
            [](Encoder &encoder, ModuleRef module, const std::string &name,
               const std::vector<DescriptorSetInfoRef> &descriptors, const std::vector<BindingSlotRef> &inputs,
               const std::vector<BindingSlotRef> &outputs, const std::vector<ConstantRef> &constants,
               const std::array<uint32_t, 3> &dispatchShape, const std::vector<PushConstRangeRef> &pushConstRanges) {
                std::vector<GraphConstantBindingRef> constantBindings;
                constantBindings.reserve(constants.size());
                std::copy(constants.begin(), constants.end(), std::back_inserter(constantBindings));
                return encoder.AddSegmentInfo(module, name, descriptors, inputs, outputs, constantBindings,
                                              dispatchShape, pushConstRanges);
            },
            "Add a segment using legacy constant references.", py::arg("module"), py::arg("name"),
            py::arg("descriptors") = py::list(), py::arg("inputs") = py::list(), py::arg("outputs") = py::list(),
            py::arg("constants") = py::list(), py::arg("dispatchShape") = std::array<uint32_t, 3>(),
            py::arg("pushConstRanges") = py::list())
        .def("AddSegmentInfo",
             py::overload_cast<ModuleRef, const std::string &, const std::vector<DescriptorSetInfoRef> &,
                               const std::vector<BindingSlotRef> &, const std::vector<BindingSlotRef> &,
                               const std::vector<GraphConstantBindingRef> &, const std::array<uint32_t, 3> &,
                               const std::vector<PushConstRangeRef> &>(&Encoder::AddSegmentInfo),
             "Add a segment using explicit graph constant bindings.", py::arg("module"), py::arg("name"),
             py::arg("descriptors"), py::arg("inputs"), py::arg("outputs"), py::arg("constantBindings"),
             py::arg("dispatchShape") = std::array<uint32_t, 3>(), py::arg("pushConstRanges") = py::list())
        .def("AddModelSequenceInputsOutputs", &Encoder::AddModelSequenceInputsOutputs,
             "Set the model-level input and output binding slots and optional names.", py::arg("inputs") = py::list(),
             py::arg("inputNames") = py::list(), py::arg("outputs") = py::list(), py::arg("outputNames") = py::list())
        .def("AddInputResource", &Encoder::AddInputResource, "Add an input to the model resource table.",
             py::arg("vkDescriptorType"), py::arg("vkFormat"), py::arg("shape"), py::arg("strides"),
             py::arg("aliasGroupId") = py::none())
        .def("AddOutputResource", &Encoder::AddOutputResource, "Add an output to the model resource table.",
             py::arg("vkDescriptorType"), py::arg("vkFormat"), py::arg("shape"), py::arg("strides"),
             py::arg("aliasGroupId") = py::none())
        .def("AddIntermediateResource", &Encoder::AddIntermediateResource,
             "Add an intermediate value to the model resource table.", py::arg("vkDescriptorType"), py::arg("vkFormat"),
             py::arg("shape"), py::arg("strides"), py::arg("aliasGroupId") = py::none())
        .def("AddConstantResource", &Encoder::AddConstantResource,
             "Add a constant resource to the model resource table.", py::arg("vkFormat"), py::arg("shape"),
             py::arg("strides"))
        .def("AddSamplerConfig", &Encoder::AddSamplerConfig, "Set sampler metadata for a model resource.",
             py::arg("resource"), py::arg("samplerMinFilter"), py::arg("samplerMagFilter"),
             py::arg("samplerAddressModeU"), py::arg("samplerAddressModeV"), py::arg("samplerBorderColor"))
        .def("SetAliasGroup", &Encoder::SetAliasGroup, "Assign an alias group to a non-constant model resource.",
             py::arg("resource"), py::arg("aliasGroupId"))
        .def(
            "AddConstant",
            [](Encoder &encoder, ResourceRef resRef, const py::buffer &buffer, int64_t sparsityDimension) {
                return encoder.AddConstant(resRef, buffer.request().ptr,
                                           size_t(buffer.request().itemsize) * size_t(buffer.request().size),
                                           sparsityDimension);
            },
            "Add buffer-protocol data for a constant model resource.", py::arg("resourceRef"), py::arg("buffer"),
            py::arg("sparsityDimension") = CONSTANT_NOT_SPARSE_DIMENSION)
        .def("Finish", &Encoder::Finish, "Finish encoding the VGF file before writing it.")
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
            "Write the finished VGF file to a binary Python IO stream.", py::arg("output"));

    m.def("CreateEncoder", &CreateEncoder, "Create an encoder using the supplied Vulkan header version.",
          py::arg("vkHeaderVersion"));
}
