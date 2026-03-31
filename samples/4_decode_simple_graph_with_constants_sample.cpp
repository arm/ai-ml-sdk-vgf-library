/*
 * SPDX-FileCopyrightText: Copyright 2024-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "samples.hpp"

#include <algorithm>

namespace mlsdk::vgflib::samples {

void T4_decode_simple_graph_with_constants_sample(const std::string &vgfFilename) {
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
    std::vector<char> header(headerSize);
    vgfFile.read(header.data(), static_cast<std::streamsize>(header.size()));
    assert(vgfFile);

    // Lets create an object to decode the bytes we have just read from file.
    std::unique_ptr<vgflib::HeaderDecoder> headerDecoder =
        vgflib::CreateHeaderDecoder(header.data(), static_cast<uint64_t>(header.size()), fileSize);

    // Validate that decoding succeeded
    assert(headerDecoder);

    // ** NEW ** Create a space to store the information we need
    std::vector<uint32_t> cachedIndexes;
    {
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

        // Create the decoder for the Model Sequence Table
        std::unique_ptr<vgflib::ModelSequenceTableDecoder> mstDecoder =
            vgflib::CreateModelSequenceTableDecoder(modelSequenceTableData.data(), modelSequenceTableData.size());

        // Check for valid section
        assert(mstDecoder != nullptr);

        // We know this file was written based on tutorial 3, so lets verify some things we expect.
        assert(mstDecoder->modelSequenceTableSize() == 1);                       // We should only have 1 segment
        uint32_t segIdx = 0;                                                     // Therefore it is index 0
        assert(mstDecoder->getSegmentType(segIdx) == vgflib::ModuleType::GRAPH); // It should be a SPIR-V graph module
        assert(mstDecoder->getSegmentName(segIdx) ==
               "segment_conv2d_rescale_graph2");                           // We know what it should be named
        assert(mstDecoder->getSegmentDescriptorSetInfosSize(segIdx) == 1); // Only one descriptor set used.
        // ...etc

        // ** NEW ** Unlike tutorial 2, there should be constants this time
        vgflib::DataView<uint32_t> indexes = mstDecoder->getSegmentConstantIndexes(segIdx);
        assert(indexes.size() == 1); // we expect 1 constant required by this segment

        // ** NEW ** Each element in indexes, is an index to an entry the constants table.
        // ** NEW ** We will cache the indexes for later, since the DataView will dangle once we leave the current
        // scope.
        cachedIndexes.reserve(indexes.size());
        std::for_each(indexes.begin(), indexes.end(), [&cachedIndexes](uint32_t v) { cachedIndexes.push_back(v); });
    }

    //===================
    // ** NEW (BEGIN) **
    //===================
    {
        // Now we will load the constants from the file. For this tutorial we will simply load the entire
        // constants section into memory, though more efficient strategies exist, and will be covered in
        // a later tutorial.

        // Load the constants section from the VGF
        std::vector<char> constantsData(headerDecoder->GetConstantsSize());
        vgfFile.seekg(static_cast<std::streamoff>(headerDecoder->GetConstantsOffset()));
        vgfFile.read(constantsData.data(), static_cast<std::streamsize>(constantsData.size()));
        assert(vgfFile);

        // Create the ConstantsDecoder from the section in memory
        std::unique_ptr<vgflib::ConstantDecoder> constantsDecoder =
            vgflib::CreateConstantDecoder(constantsData.data(), constantsData.size());

        // Check for valid section
        assert(constantsDecoder != nullptr);

        assert(constantsDecoder->size() == 1); // There should be 1 constant
        [[maybe_unused]] vgflib::DataView<uint8_t> weightsBytes =
            constantsDecoder->getConstant(0);                                 // Index 0 is weights
        assert(static_cast<int32_t>(weightsBytes.size()) == 16 * 2 * 2 * 16); // Weights kernel size is 16x2x2x16
        //=================
        // ** NEW (END) **
        //=================
    }
}

} // namespace mlsdk::vgflib::samples
