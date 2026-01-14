/*
 * SPDX-FileCopyrightText: Copyright 2024-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "samples.hpp"

#include <algorithm>

namespace mlsdk::vgflib::samples {

void T4_decode_simple_graph_with_constants_sample(const std::string &vgfFilename) {
    // Open the file
    std::ifstream vgf_file(vgfFilename, std::ios::binary);
    assert(vgf_file);
    size_t header_size = vgflib::HeaderSize();

    // get length of file so we can do some error checking
    vgf_file.seekg(0, std::ios_base::end);
    assert(header_size <= static_cast<size_t>(vgf_file.tellg()));
    vgf_file.seekg(0, std::ios_base::beg);

    // Read exactly 'headerSize' num bytes of data
    std::vector<char> header(header_size);
    vgf_file.read(header.data(), static_cast<std::streamsize>(header.size()));
    assert(vgf_file);

    // Lets create an object to decode the bytes we have just read from file.
    std::unique_ptr<vgflib::HeaderDecoder> header_decoder =
        vgflib::CreateHeaderDecoder(header.data(), static_cast<uint64_t>(header.size()));

    // Check that the header has decoded a valid VGF file
    assert(header_decoder->IsValid());

    // Check that the Version of the VGF is compatible with the VGF decoder library being used
    // In other words, Major version must match and minor version of the API should be same or newer than file
    assert(header_decoder->CheckVersion());

    // ** NEW ** Create a space to store the information we need
    std::vector<uint32_t> cached_indexes;
    {
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

        // Check for valid section
        assert(vgflib::VerifyModelSequenceTable(model_sequence_table_data.data(), model_sequence_table_data.size()));

        // Create the decoder for the Model Sequence Table
        std::unique_ptr<vgflib::ModelSequenceTableDecoder> mst_decoder =
            vgflib::CreateModelSequenceTableDecoder(model_sequence_table_data.data(), model_sequence_table_data.size());

        // We know this file was written based on tutorial 3, so lets verify some things we expect.
        assert(mst_decoder->modelSequenceTableSize() == 1);                       // We should only have 1 segment
        uint32_t segIdx = 0;                                                      // Therefore it is index 0
        assert(mst_decoder->getSegmentType(segIdx) == vgflib::ModuleType::GRAPH); // It should be a SPIR-V graph module
        assert(mst_decoder->getSegmentName(segIdx) ==
               "segment_conv2d_rescale_graph2");                            // We know what it should be named
        assert(mst_decoder->getSegmentDescriptorSetInfosSize(segIdx) == 1); // Only one descriptor set used.
        // ...etc

        // ** NEW ** Unlike tutorial 2, there should be constants this time
        vgflib::DataView<uint32_t> indexes = mst_decoder->getSegmentConstantIndexes(segIdx);
        assert(indexes.size() == 1); // we expect 1 constant required by this segment

        // ** NEW ** Each element in indexes, is an index to an entry the constants table.
        // ** NEW ** We will cache the indexes for later, since the DataView will dangle once we leave the current
        // scope.
        cached_indexes.reserve(indexes.size());
        std::for_each(indexes.begin(), indexes.end(), [&cached_indexes](uint32_t v) { cached_indexes.push_back(v); });
    }

    //===================
    // ** NEW (BEGIN) **
    //===================
    {
        // Now we will load the constants from the file. For this tutorial we will simply load the entire
        // constants section into memory, though more efficient strategies exist, and will be covered in
        // a later tutorial.

        // Load the constants section from the VGF
        std::vector<char> constants_data(header_decoder->GetConstantsSize());
        vgf_file.seekg(static_cast<std::streamoff>(header_decoder->GetConstantsOffset()));
        vgf_file.read(constants_data.data(), static_cast<std::streamsize>(constants_data.size()));
        assert(vgf_file);

        // Check for valid section
        assert(vgflib::VerifyConstant(constants_data.data(), constants_data.size()));

        // Create the ConstantsDecoder from the section in memory
        std::unique_ptr<vgflib::ConstantDecoder> constants_decoder =
            vgflib::CreateConstantDecoder(constants_data.data(), constants_data.size());

        assert(constants_decoder->size() == 1); // There should be 1 constant
        [[maybe_unused]] vgflib::DataView<uint8_t> weights_bytes =
            constants_decoder->getConstant(0);                                 // Index 0 is weights
        assert(static_cast<int32_t>(weights_bytes.size()) == 16 * 2 * 2 * 16); // Weights kernel size is 16x2x2x16
        //=================
        // ** NEW (END) **
        //=================
    }
}

} // namespace mlsdk::vgflib::samples
