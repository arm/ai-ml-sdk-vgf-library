/*
 * SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "vgf-utils/binding_slot.hpp"

#include <cstdint>
#include <vector>

namespace mlsdk::vgflib {
class ModelSequenceTableDecoder;
class ModelResourceTableDecoder;
} // namespace mlsdk::vgflib

namespace mlsdk::vgfutils {

/** @brief A model-interface binding. */
struct ModelInterfaceEntry {
    BindingSlot bindingSlot;
    /** @brief Zero-based resource-table order among inputs and outputs, skipping constants and intermediates. */
    uint32_t inputOutputIndex{0};
};

/** @brief Model-interface entries in their declared input and output order. */
struct ModelInterface {
    std::vector<ModelInterfaceEntry> inputs;
    std::vector<ModelInterfaceEntry> outputs;
};

/** @brief Reads model-interface bindings in their declared input and output order. */
ModelInterface parseModelInterface(const vgflib::ModelSequenceTableDecoder &sequence,
                                   const vgflib::ModelResourceTableDecoder &resources);

} // namespace mlsdk::vgfutils
