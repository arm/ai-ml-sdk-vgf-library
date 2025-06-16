/*
 * SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "samples.hpp"

#include <filesystem>

namespace mlsdk::vgflib::samples {
std::string T1_encode_simple_graph_sample() {
    // This SPIR-V disassembly defines a graph with a single MaxPool2D TOSA operator.
    const std::string single_maxpool_graph =
        R""(
               OpCapability ReplicatedCompositesEXT
               OpCapability TensorsARM
               OpCapability Int8
               OpCapability GraphARM
               OpCapability Shader
               OpCapability VulkanMemoryModel
               OpCapability Int16
               OpCapability Int64
               OpCapability Matrix
               OpExtension "SPV_EXT_replicated_composites"
               OpExtension "SPV_ARM_tensors"
               OpExtension "SPV_ARM_graph"
               OpExtension "SPV_KHR_vulkan_memory_model"
         %29 = OpExtInstImport "TOSA.001000.1"
               OpMemoryModel Logical Vulkan
               OpName %graph_partition_0_arg_0 "graph_partition_0_arg_0"
               OpName %graph_partition_0_res_0 "graph_partition_0_res_0"
               OpDecorate %graph_partition_0_arg_0 Binding 0
               OpDecorate %graph_partition_0_arg_0 DescriptorSet 0
               OpDecorate %graph_partition_0_res_0 Binding 1
               OpDecorate %graph_partition_0_res_0 DescriptorSet 0
      %uchar = OpTypeInt 8 0
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_uint_uint_4 = OpTypeArray %uint %uint_4
     %uint_1 = OpConstant %uint 1
    %uint_16 = OpConstant %uint 16
          %7 = OpConstantComposite %_arr_uint_uint_4 %uint_1 %uint_16 %uint_16 %uint_16
          %2 = OpTypeTensorARM %uchar %uint_4 %7
%_ptr_UniformConstant_2 = OpTypePointer UniformConstant %2
%graph_partition_0_arg_0 = OpVariable %_ptr_UniformConstant_2 UniformConstant
     %uint_8 = OpConstant %uint 8
         %13 = OpConstantComposite %_arr_uint_uint_4 %uint_1 %uint_8 %uint_8 %uint_16
         %12 = OpTypeTensorARM %uchar %uint_4 %13
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
%graph_partition_0_res_0 = OpVariable %_ptr_UniformConstant_12 UniformConstant
         %17 = OpTypeGraphARM 1 %2 %12
     %uint_0 = OpConstant %uint 0
%_arr_uint_uint_1 = OpTypeArray %uint %uint_1
     %uint_2 = OpConstant %uint 2
         %22 = OpConstantComposite %_arr_uint_uint_1 %uint_2
         %20 = OpTypeTensorARM %uint %uint_1 %22
         %24 = OpConstantCompositeReplicateEXT %20 %uint_2
         %26 = OpConstantComposite %_arr_uint_uint_1 %uint_4
         %25 = OpTypeTensorARM %uint %uint_1 %26
         %27 = OpConstantNull %25
               OpGraphEntryPointARM %16 "graph_partition_0" %graph_partition_0_arg_0 %graph_partition_0_res_0
         %16 = OpGraphARM %17
         %18 = OpGraphInputARM %2 %uint_0
         %28 = OpExtInst %12 %29 MAX_POOL2D %24 %24 %27 %uint_1 %18
               OpGraphSetOutputARM %28 %uint_0
               OpGraphEndARM

)"";

    // For the purpose of this sample, we construct the SPIR-V binary from the above SPIR-V disassembly.
    // In production, the code will probably be loaded as SPIR-V binary directly from a .spv file.
    // The SPIR-V code specifically defines a graph type module rather than a compute shader. This will be relevant
    // later in this sample. Assemble the binary from the SPIR-V disassembly
    std::vector<uint32_t> spirv_code = sampleutils::spirv_assemble(single_maxpool_graph);

    // First, lets create an encoder object. This will be used to construct the internal VGF
    // structures before serializing to disk.
    std::unique_ptr<vgflib::Encoder> encoder = vgflib::CreateEncoder(VK_HEADER_VERSION);

    // Add a module based on the SPIR-V binary encoding
    // Two types are currently supported. Graph or Compute shader modules
    vgflib::ModuleRef graph_ref =
        encoder->AddModule(vgflib::ModuleType::GRAPH, // Explicitly call out this specific module type as Graph. This
                                                      // needs to match the type of the supplied SPIR-V code.
                           "single_maxpool", // Give the module a name. Any name that can help with identification for
                                             // debug/tooling purposes.
                           "main",    // the name of the module entry function, as defined by the OpGraphEntryPointEXT
                                      // SPIR-V instruction in the case of a graph.
                           spirv_code // The SPIR-V binary code for the module
        );

    // Define the resources that will be used by the whole model. This adds entries into the ModelResourceTable.
    // Note: It is through these entries that graph connectivity between segments (see below) can be deduced if
    // required.
    std::vector<int64_t> input_shape = {1, 16, 16, 16};
    vgflib::ResourceRef input_res_ref =
        encoder->AddInputResource( // this resource is going to be an INPUT to the whole model
            vgflib::ToDescriptorType(VK_DESCRIPTOR_TYPE_TENSOR_ARM), // it will be a vulkan storage tensor
                                                                     // resource type
            vgflib::ToFormatType(VK_FORMAT_R8_SINT),                 // the elements will be int8 type
            input_shape,                                             // 1x16x16x16 input shape
            {});

    // Note: It is up to the user to ensure that the resources are compatible with the attached modules.
    std::vector<int64_t> output_shape = {1, 8, 8, 16};
    vgflib::ResourceRef output_res_ref =
        encoder->AddOutputResource( // this resource is going to be an OUTPUT to the whole model
            vgflib::ToDescriptorType(VK_DESCRIPTOR_TYPE_TENSOR_ARM), // it will be a vulkan storage tensor
                                                                     // resource type
            vgflib::ToFormatType(VK_FORMAT_R8_SINT),                 // the elements will be int8 type
            output_shape,                                            // 1x8x8x16 input shape after a 2x2 MaxPool2D op
            {});

    // Define the binding slots that are ultimately used by DescriptorSets. This method pairs the binding slot to a
    // resource in the ModelResourceTable. A single resource can be referenced by many binding slots. This particular
    // slot is used for the whole model input which is ALSO the segment input in this case
    vgflib::BindingSlotRef input_binding_ref = encoder->AddBindingSlot(
        0,            // binding id 0 is the descriptor set binding id expected for the INPUT of the graph above
        input_res_ref // the info (shape, type, etc) about the resource that will be bound at runtime
    );

    // This slot is used for the whole model output which is ALSO the segment output in this case
    vgflib::BindingSlotRef output_binding_ref = encoder->AddBindingSlot(
        1,             // binding id 1 is the VkDescriptorSet binding id for the OUTPUT of the example graph above
        output_res_ref // the info (shape, type, etc) about the resource that will be bound at runtime
    );

    // Input and output are bound to a single descriptor set.
    // DescriptorSets are added contiguously, in order so for a shader where "set=0", the corresponding descriptor set
    // should be the first one added to the encoder and where "set=1", it should be the second, etc.
    vgflib::DescriptorSetInfoRef desc_info_ref = encoder->AddDescriptorSetInfo({
        input_binding_ref, // Input binding slot (id=0)
        output_binding_ref // Output binding slot (id=1)
    });

    // Add a segment. The order in which they are added to the encoder is the order in which they should be executed.
    encoder->AddSegmentInfo(graph_ref,                // the graph module
                            "segment_maxpool_graph1", // the name of the segment for debug/tooling purposes
                            {desc_info_ref},          // the descriptor sets used by the model
                            {input_binding_ref},  // the input bindings of the segment. Linked resource can be either
                                                  //   ResourceCategory INPUT or INTERMEDIATE
                            {output_binding_ref}, // the output binding of the segment. Linked resource can be either
                                                  //   ResourceCategory OUTPUT or INTERMEDIATE
                            {}, // this graph module has only one MaxPool2D layer so doesn't make use of any constants
                            {}, // this segment is for a graph module so doesn't require any dispatch shape
                            {}  // this example does not make use of push constants.
    );

    // Define the inputs and outputs for the model. In this simple case, they are the same as the inputs and outputs to
    // the segment.
    encoder->AddModelSequenceInputsOutputs(
        {input_binding_ref},  // the input bindings of the whole model. Should all be of type INPUT
        {"input_0"},          // name of the input so that consumers can bind the input data correctly at runtime.
        {output_binding_ref}, // the output binding of the whole model. Should all be of type OUTPUT
        {"output_0"}          // name of the output so that consumers can bind the output data correctly at runtime.
    );

    // Mark the encoder as done to finalize any processing before serialization
    encoder->Finish();

    // Write the contents to any valid ostream
    std::string filename = "single_maxpool_graph.vgf";
    std::filesystem::path full_path = std::filesystem::temp_directory_path();
    full_path /= filename;

    std::ofstream vgf_file(full_path.c_str(), std::ofstream::binary | std::ofstream::trunc);
    encoder->WriteTo(vgf_file);
    vgf_file.close();

    // Now that the file has been written to disk, you can view the contents via the VGF_dump tool which is a
    // stand-alone executable and built as part of the VGF library. See: <SDK_ROOT>/format/vgf_dump/

    return full_path.string();
}
} // namespace mlsdk::vgflib::samples
