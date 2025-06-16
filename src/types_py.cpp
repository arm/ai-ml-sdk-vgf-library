/*
 * SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf/types.hpp"

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

using namespace mlsdk::vgflib;

void pyInitTypes(py::module m) {

    py::enum_<ModuleType>(m, "ModuleType").value("Compute", ModuleType::COMPUTE).value("Graph", ModuleType::GRAPH);

    py::enum_<ResourceCategory>(m, "ResourceCategory")
        .value("Input", ResourceCategory::INPUT)
        .value("Output", ResourceCategory::OUTPUT)
        .value("Intermediate", ResourceCategory::INTERMEDIATE)
        .value("Constant", ResourceCategory::CONSTANT);

    py::class_<FourCCValue>(m, "FourCCValue")
        .def(py::init<char, char, char, char>())
        .def(py::self == py::self)
        .def_readwrite("a", &FourCCValue::a)
        .def_readwrite("b", &FourCCValue::b)
        .def_readwrite("c", &FourCCValue::c)
        .def_readwrite("d", &FourCCValue::d);

    m.def("FourCC", &FourCC, py::arg("a"), py::arg("b"), py::arg("c"), py::arg("d"));

    m.def("UndefinedFormat", &UndefinedFormat);
}
