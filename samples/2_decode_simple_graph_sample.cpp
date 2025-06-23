/*
 * SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "samples.hpp"

#include <fstream>
#include <iostream>
#include <memory>

namespace mlsdk::vgflib::samples {

void T2_decode_simple_graph_sample(const std::string &vgfFilename) {

    // Open the file
    std::ifstream vgf_file(vgfFilename, std::ios::binary);
    assert(vgf_file);

    // get length of file so we can do some error checking
    vgf_file.seekg(0, std::ios_base::end);
    std::streampos file_size = vgf_file.tellg();
    vgf_file.seekg(0, std::ios_base::beg);

    size_t header_size = vgflib::HeaderSize();
    assert(header_size <= static_cast<size_t>(file_size));

    // Read exactly 'headerSize' num bytes of data
    std::vector<char> header_mem(header_size);
    vgf_file.read(header_mem.data(), static_cast<std::streamsize>(header_mem.size()));
    assert(vgf_file);

    // Lets create an object to decode the bytes we have just read from file.
    std::unique_ptr<vgflib::HeaderDecoder> header_decoder = vgflib::CreateHeaderDecoder(header_mem.data());

    // Check that the header has decoded a valid VGF file
    assert(header_decoder->IsValid());

    // Check that the Version of the VGF is compatible with the VGF decoder library being used
    // In other words, Major version must match and minor version of the API should be same or newer than file
    assert(header_decoder->CheckVersion());

    // Lets first load the segment info which is part of the ModelSequenceTable section.
    // The header can tell us exactly where this section is in the file and how large it is.
    // For this example we will load the entire section into memory, though it is possible to memory map the file
    // and access the contents directly.
    std::vector<char> model_sequence_table_data(header_decoder->GetModelSequenceTableSize());

    // Seek to the position in the file where the section begins
    vgf_file.seekg(static_cast<std::streamoff>(header_decoder->GetModelSequenceTableOffset()));

    // Read the bytes into our allocation
    vgf_file.read(model_sequence_table_data.data(), static_cast<std::streamsize>(model_sequence_table_data.size()));

    // Make sure nothing went wrong
    assert(vgf_file);

    // and now create the object that will decode the contents of the section for us.
    std::unique_ptr<vgflib::ModelSequenceTableDecoder> mst_decoder =
        vgflib::CreateModelSequenceTableDecoder(model_sequence_table_data.data());

    // We know this file was written based on tutorial 1, so lets verify some things we expect.
    assert(mst_decoder->modelSequenceTableSize() == 1);                       // We should only have 1 segment
    uint32_t segIdx = 0;                                                      // Therefore it is index 0
    assert(mst_decoder->getSegmentType(segIdx) == vgflib::ModuleType::GRAPH); // It should be a SPIR-V graph module
    assert(mst_decoder->getSegmentName(segIdx) == "segment_maxpool_graph1");  // We know what it should be named
    assert(mst_decoder->getSegmentConstantIndexes(segIdx).size() == 0);       // There are no graph constants used.
    assert(mst_decoder->getSegmentDescriptorSetInfosSize(segIdx) == 1);       // Only one descriptor set used.
    // ...etc

    uint32_t descIdx = 0;
    vgflib::BindingSlotArrayHandle desc_slots = mst_decoder->getDescriptorBindingSlotsHandle(segIdx, descIdx);
    assert(mst_decoder->getBindingsSize(desc_slots) == 2); // There are two bindings on the DescriptorSet

    // Get the binding id for each slot by index
    // Binding ids don't need to be contiguous though they are in this case and just happen to match the indexes
    assert(mst_decoder->getBindingSlotBinding(desc_slots, 0) == 0);
    assert(mst_decoder->getBindingSlotBinding(desc_slots, 1) == 1);

    // We happen to know, because we wrote the VGF in tutorial 1 that desc_slot 0 is the input and 1 is the output
    // so lets name the variables accordingly for the sake of this sample code.
    uint32_t input_mrt_idx = mst_decoder->getBindingSlotMrtIndex(desc_slots, 0);
    uint32_t output_mrt_idx = mst_decoder->getBindingSlotMrtIndex(desc_slots, 1);

    // Now that we have extracted and cached all the info we presumably need from the ModelSequenceTable, lets free up
    // the memory so we can load up the next section

    // Destroy the decoder object before releasing the section memory.
    // Note this ordering would naturally occur once we left scope
    mst_decoder.reset();
    assert(!mst_decoder);

    // Release the section memory
    // Note if we were mmapping the section, we would unmap at this point.
    model_sequence_table_data.clear(); // Release the section memory
    assert(model_sequence_table_data.empty());

    {
        // We will now load the resource info from the ModelResourceTable
        // We have already found and cached the indices for the input and output resources.
        // So lets start by creating the decoder for the ModelresourceTable section.
        std::vector<char> model_resource_table_data(header_decoder->GetModelResourceTableSize());
        vgf_file.seekg(static_cast<std::streamoff>(header_decoder->GetModelResourceTableOffset()));
        vgf_file.read(model_resource_table_data.data(), static_cast<std::streamsize>(model_resource_table_data.size()));
        assert(vgf_file);

        // and now create the object that will decode the MRT contents for us.
        std::unique_ptr<vgflib::ModelResourceTableDecoder> mrt_decoder =
            vgflib::CreateModelResourceTableDecoder(model_resource_table_data.data());

        // All the fields are trivially accessed
        assert(mrt_decoder->size() == 2); // 2 resources, 1 input and 1 output for this use case

        // Get the input resource fields
        assert(mrt_decoder->getCategory(input_mrt_idx) ==
               vgflib::ResourceCategory::INPUT); // Index 0 is the input resource
        [[maybe_unused]] std::optional<vgflib::DescriptorType> descTypeIn =
            mrt_decoder->getDescriptorType(input_mrt_idx);
        assert(descTypeIn.has_value());
        assert(descTypeIn.value_or(vgflib::DescriptorType(-1)) == VK_DESCRIPTOR_TYPE_TENSOR_ARM); // Tensor
        assert(mrt_decoder->getVkFormat(input_mrt_idx) == VK_FORMAT_R8_SINT);                     // Is int8 type
        std::vector<int64_t> ins = {1, 16, 16, 16};
        vgflib::DataView<int64_t> inShape(ins.data(), ins.size());
        assert(mrt_decoder->getTensorShape(input_mrt_idx) == inShape); // Input shape is 1x16x16x16

        // Get the output resource fields
        assert(mrt_decoder->getCategory(output_mrt_idx) ==
               vgflib::ResourceCategory::OUTPUT); // Index 1 is the output resource
        [[maybe_unused]] std::optional<vgflib::DescriptorType> descTypeOut =
            mrt_decoder->getDescriptorType(output_mrt_idx);
        assert(descTypeOut.has_value());
        assert(descTypeOut.value_or(vgflib::DescriptorType(-1)) == VK_DESCRIPTOR_TYPE_TENSOR_ARM); // Tensor
        assert(mrt_decoder->getVkFormat(input_mrt_idx) == VK_FORMAT_R8_SINT);                      // Is int8 type
        std::vector<int64_t> outs = {1, 8, 8, 16};
        vgflib::DataView<int64_t> outShape(outs.data(), outs.size());
        assert(mrt_decoder->getTensorShape(input_mrt_idx) == outShape); // Output shape is 1x8x8x16

        // going out of scope will automatically free the modelResourceTableData and mrt decoder in the correct/safe
        // order note that any DataViews or string_views that persist after the modelResourceTableData is freed, will be
        // invalid. It is therefore recommended to make copies where these might outlive the mmapped or loaded file
        // data.
    }

    // For the next section, we will use the optional in-place decoder factory method variant for illustrative purposes.
    // This API is provided for users who wish to explicitly control which custom heap is used for section decoding.
    // We will demonstrate this usage to decode the graph module.
    {
        std::vector<char> module_table_data(header_decoder->GetModuleTableSize());
        vgf_file.seekg(static_cast<std::streamoff>(header_decoder->GetModuleTableOffset()));
        vgf_file.read(module_table_data.data(), static_cast<std::streamsize>(module_table_data.size()));
        assert(vgf_file);

        // *NEW* Allocate some memory to hold the decoder object from a heap of your choice.
        std::vector<char> decoder_mem(vgflib::ModuleTableDecoderSize());

        // *NEW* create the section decoder but this time using the in-place factory method variant.
        // modules_decoder is constructed inside the 'decoderMem.data()' allocation.
        vgflib::ModuleTableDecoder *modules_decoder =
            vgflib::CreateModuleTableDecoderInPlace(module_table_data.data(), decoder_mem.data());

        // access the fields as required
        assert(modules_decoder->size() == 1);                          // Only one graph module
        assert(modules_decoder->getModuleName(0) == "single_maxpool"); // We named it when encoding the file in lesson 1

        // The following fields will be required in order to create the Vulkan compute or graph pipelines
        assert(modules_decoder->getModuleType(0) == vgflib::ModuleType::GRAPH); // is a graph module
        assert(modules_decoder->hasSPIRV(0) == true); // source is included and it's not a placeholder module.
                                                      // a placeholder module would require the user to provision the
                                                      // source code via some other/bespoke route.
        assert(modules_decoder->getModuleEntryPoint(0) == "main"); // the entry function name
        assert(modules_decoder->getModuleCode(0).size() > 0);      // the source code, if embedded

        // Once the pipelines have been compiled, we don't need the modules to be loaded.
        // explicitly destruct the decoder now that we're done with it
        modules_decoder->~ModuleTableDecoder();
    }
}

} // namespace mlsdk::vgflib::samples
