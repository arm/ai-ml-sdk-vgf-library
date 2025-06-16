/*
 * SPDX-FileCopyrightText: Copyright 2024 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

extern void pyInitTypes(py::module m);
extern void pyInitEncoder(py::module m);
extern void pyInitDecoder(py::module m);

PYBIND11_MODULE(vgfpy, m) {
    pyInitTypes(m);
    pyInitEncoder(m);
    pyInitDecoder(m);
}
