/*
 * SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf-utils/model_interface.hpp"

#include "vgf/decoder.hpp"
#include "vgf/types.hpp"

#include <limits>
#include <stdexcept>
#include <vector>

namespace mlsdk::vgfutils {
namespace {

bool isInputOrOutput(vgflib::ResourceCategory category) {
    return category == vgflib::ResourceCategory::INPUT || category == vgflib::ResourceCategory::OUTPUT;
}

std::vector<ModelInterfaceEntry> parseBindings(const vgflib::ModelSequenceTableDecoder &sequence,
                                               vgflib::BindingSlotArrayHandle handle,
                                               const vgflib::ModelResourceTableDecoder &resources,
                                               const std::vector<uint32_t> &inputOutputIndices,
                                               vgflib::ResourceCategory expectedCategory) {
    std::vector<ModelInterfaceEntry> bindings;
    const auto count = sequence.getBindingsSize(handle);
    bindings.reserve(count);
    for (uint32_t slot = 0; slot < count; ++slot) {
        const auto mrtIndex = sequence.getBindingSlotMrtIndex(handle, slot);
        if (mrtIndex >= resources.size() || resources.getCategory(mrtIndex) != expectedCategory) {
            throw std::runtime_error("VGF model interface binding references an invalid resource");
        }
        bindings.push_back(
            {BindingSlot(slot, sequence.getBindingSlotBinding(handle, slot), mrtIndex), inputOutputIndices[mrtIndex]});
    }
    return bindings;
}

} // namespace

ModelInterface parseModelInterface(const vgflib::ModelSequenceTableDecoder &sequence,
                                   const vgflib::ModelResourceTableDecoder &resources) {
    if (resources.size() > std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("VGF model resource table is too large");
    }
    std::vector<uint32_t> inputOutputIndices(resources.size());
    uint32_t inputOutputCount = 0;
    for (uint32_t mrtIndex = 0; mrtIndex < resources.size(); ++mrtIndex) {
        if (isInputOrOutput(resources.getCategory(mrtIndex))) {
            inputOutputIndices[mrtIndex] = inputOutputCount++;
        }
    }

    return {parseBindings(sequence, sequence.getModelSequenceInputBindingSlotsHandle(), resources, inputOutputIndices,
                          vgflib::ResourceCategory::INPUT),
            parseBindings(sequence, sequence.getModelSequenceOutputBindingSlotsHandle(), resources, inputOutputIndices,
                          vgflib::ResourceCategory::OUTPUT)};
}

} // namespace mlsdk::vgfutils
