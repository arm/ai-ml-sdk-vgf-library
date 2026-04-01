/*
 * SPDX-FileCopyrightText: Copyright 2024-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "samples.hpp"

#include <fstream>
#include <iostream>
#include <memory>

namespace mlsdk::vgflib::samples {

void T2_decode_simple_graph_sample(const std::string &vgfFilename) {

    // Open the file
    std::ifstream vgfFile(vgfFilename, std::ios::binary);
    assert(vgfFile);
    size_t headerSize = vgflib::HeaderSize();

    // get length of file so we can do some error checking
    vgfFile.seekg(0, std::ios_base::end);
    const uint64_t fileSize = static_cast<uint64_t>(vgfFile.tellg());
    assert(headerSize <= static_cast<size_t>(fileSize));
    vgfFile.seekg(0, std::ios_base::beg);

    // Read exactly 'headerSize' num bytes of data
    std::vector<char> headerMem(headerSize);
    vgfFile.read(headerMem.data(), static_cast<std::streamsize>(headerMem.size()));
    assert(vgfFile);

    // Lets create an object to decode the bytes we have just read from file.
    std::unique_ptr<vgflib::HeaderDecoder> headerDecoder =
        vgflib::CreateHeaderDecoder(headerMem.data(), static_cast<uint64_t>(headerMem.size()), fileSize);

    // Validate that decoding succeeded
    assert(headerDecoder);

    // Lets first load the segment info which is part of the ModelSequenceTable section.
    // The header can tell us exactly where this section is in the file and how large it is.
    // For this example we will load the entire section into memory, though it is possible to memory map the file
    // and access the contents directly.
    std::vector<char> modelSequenceTableData(headerDecoder->GetModelSequenceTableSize());

    // Seek to the position in the file where the section begins
    vgfFile.seekg(static_cast<std::streamoff>(headerDecoder->GetModelSequenceTableOffset()));

    // Read the bytes into our allocation
    vgfFile.read(modelSequenceTableData.data(), static_cast<std::streamsize>(modelSequenceTableData.size()));

    // Make sure nothing went wrong
    assert(vgfFile);

    // Now create the object that will decode the contents of the section for us.
    std::unique_ptr<vgflib::ModelSequenceTableDecoder> mstDecoder =
        vgflib::CreateModelSequenceTableDecoder(modelSequenceTableData.data(), modelSequenceTableData.size());

    // Check for valid section
    assert(mstDecoder != nullptr);

    // We know this file was written based on tutorial 1, so lets verify some things we expect.
    assert(mstDecoder->modelSequenceTableSize() == 1);                       // We should only have 1 segment
    uint32_t segIdx = 0;                                                     // Therefore it is index 0
    assert(mstDecoder->getSegmentType(segIdx) == vgflib::ModuleType::GRAPH); // It should be a SPIR-V graph module
    assert(mstDecoder->getSegmentName(segIdx) == "segment_maxpool_graph1");  // We know what it should be named
    assert(mstDecoder->getSegmentConstantIndexes(segIdx).empty());           // There are no graph constants used.
    assert(mstDecoder->getSegmentDescriptorSetInfosSize(segIdx) == 1);       // Only one descriptor set used.
    // ...etc

    uint32_t descIdx = 0;
    vgflib::BindingSlotArrayHandle descSlots = mstDecoder->getDescriptorBindingSlotsHandle(segIdx, descIdx);
    assert(mstDecoder->getBindingsSize(descSlots) == 2); // There are two bindings on the DescriptorSet

    // Get the binding id for each slot by index
    // Binding ids don't need to be contiguous though they are in this case and just happen to match the indexes
    assert(mstDecoder->getBindingSlotBinding(descSlots, 0) == 0);
    assert(mstDecoder->getBindingSlotBinding(descSlots, 1) == 1);

    // We happen to know, because we wrote the VGF in tutorial 1 that desc_slot 0 is the input and 1 is the output
    // so lets name the variables accordingly for the sake of this sample code.
    uint32_t inputMrtIdx = mstDecoder->getBindingSlotMrtIndex(descSlots, 0);
    uint32_t outputMrtIdx = mstDecoder->getBindingSlotMrtIndex(descSlots, 1);

    // Now that we have extracted and cached all the info we presumably need from the ModelSequenceTable, lets free up
    // the memory so we can load up the next section

    // Destroy the decoder object before releasing the section memory.
    // Note this ordering would naturally occur once we left scope
    mstDecoder.reset();
    assert(!mstDecoder);

    // Release the section memory
    // Note if we were mmapping the section, we would unmap at this point.
    modelSequenceTableData.clear(); // Release the section memory
    assert(modelSequenceTableData.empty());

    {
        // We will now load the resource info from the ModelResourceTable
        // We have already found and cached the indices for the input and output resources.
        // So lets start by creating the decoder for the ModelresourceTable section.
        std::vector<char> modelResourceTableData(headerDecoder->GetModelResourceTableSize());
        vgfFile.seekg(static_cast<std::streamoff>(headerDecoder->GetModelResourceTableOffset()));
        vgfFile.read(modelResourceTableData.data(), static_cast<std::streamsize>(modelResourceTableData.size()));
        assert(vgfFile);

        // Now create the object that will decode the MRT contents for us.
        std::unique_ptr<vgflib::ModelResourceTableDecoder> mrtDecoder =
            vgflib::CreateModelResourceTableDecoder(modelResourceTableData.data(), modelResourceTableData.size());

        // Check for valid section
        assert(mrtDecoder != nullptr);

        // All the fields are trivially accessed
        assert(mrtDecoder->size() == 2); // 2 resources, 1 input and 1 output for this use case

        // Get the input resource fields
        assert(mrtDecoder->getCategory(inputMrtIdx) ==
               vgflib::ResourceCategory::INPUT); // Index 0 is the input resource
        [[maybe_unused]] std::optional<vgflib::DescriptorType> descTypeIn = mrtDecoder->getDescriptorType(inputMrtIdx);
        assert(descTypeIn.has_value());
        assert(descTypeIn.value_or(vgflib::DescriptorType(-1)) == VK_DESCRIPTOR_TYPE_TENSOR_ARM); // Tensor
        assert(mrtDecoder->getVkFormat(inputMrtIdx) == VK_FORMAT_R8_SINT);                        // Is int8 type
        std::vector<int64_t> ins = {1, 16, 16, 16};
        vgflib::DataView<int64_t> inShape(ins.data(), ins.size());
        assert(mrtDecoder->getTensorShape(inputMrtIdx) == inShape); // Input shape is 1x16x16x16

        // Get the output resource fields
        assert(mrtDecoder->getCategory(outputMrtIdx) ==
               vgflib::ResourceCategory::OUTPUT); // Index 1 is the output resource
        [[maybe_unused]] std::optional<vgflib::DescriptorType> descTypeOut =
            mrtDecoder->getDescriptorType(outputMrtIdx);
        assert(descTypeOut.has_value());
        assert(descTypeOut.value_or(vgflib::DescriptorType(-1)) == VK_DESCRIPTOR_TYPE_TENSOR_ARM); // Tensor
        assert(mrtDecoder->getVkFormat(outputMrtIdx) == VK_FORMAT_R8_SINT);                        // Is int8 type
        std::vector<int64_t> outs = {1, 8, 8, 16};
        vgflib::DataView<int64_t> outShape(outs.data(), outs.size());
        assert(mrtDecoder->getTensorShape(outputMrtIdx) == outShape); // Output shape is 1x8x8x16

        // going out of scope will automatically free the modelResourceTableData and mrt decoder in the correct/safe
        // order note that any DataViews or string_views that persist after the modelResourceTableData is freed, will be
        // invalid. It is therefore recommended to make copies where these might outlive the mmapped or loaded file
        // data.
    }

    // For the next section, we will use the optional in-place decoder factory method variant for illustrative purposes.
    // This API is provided for users who wish to explicitly control which custom heap is used for section decoding.
    // We will demonstrate this usage to decode the graph module.
    {
        std::vector<char> moduleTableData(headerDecoder->GetModuleTableSize());
        vgfFile.seekg(static_cast<std::streamoff>(headerDecoder->GetModuleTableOffset()));
        vgfFile.read(moduleTableData.data(), static_cast<std::streamsize>(moduleTableData.size()));
        assert(vgfFile);

        // *NEW* Allocate some memory to hold the decoder object from a heap of your choice.
        std::vector<char> decoderMem(vgflib::ModuleTableDecoderSize());

        // *NEW* create the section decoder but this time using the in-place factory method variant.
        // modulesDecoder is constructed inside the 'decoderMem.data()' allocation.
        vgflib::ModuleTableDecoder *modulesDecoder = vgflib::CreateModuleTableDecoderInPlace(
            moduleTableData.data(), static_cast<uint64_t>(moduleTableData.size()), decoderMem.data());

        // Check for valid section
        assert(modulesDecoder != nullptr);

        // access the fields as required
        assert(modulesDecoder->size() == 1);                          // Only one graph module
        assert(modulesDecoder->getModuleName(0) == "single_maxpool"); // We named it when encoding the file in lesson 1

        // The following fields will be required in order to create the Vulkan compute or graph pipelines
        assert(modulesDecoder->getModuleType(0) == vgflib::ModuleType::GRAPH); // is a graph module
        assert(modulesDecoder->hasSPIRV(0) == true); // source is included and it's not a placeholder module.
                                                     // a placeholder module would require the user to provision the
                                                     // source code via some other/bespoke route.
        assert(modulesDecoder->getModuleEntryPoint(0) == "main"); // the entry function name
        assert(!modulesDecoder->getModuleCode(0).empty());        // the source code, if embedded

        // Once the pipelines have been compiled, we don't need the modules to be loaded.
        // explicitly destruct the decoder now that we're done with it
        modulesDecoder->~ModuleTableDecoder();
    }
}

} // namespace mlsdk::vgflib::samples
