/*
 * SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "samples.hpp"

#include <filesystem>

namespace mlsdk::vgflib::samples {
std::string T3_encode_simple_graph_with_constants_sample() {
    // This SPIR-V disassembly defines a SPIR-V graph containing a Conv2D and a Rescale operator.
    // ** NEW ** : This tutorial introduces graph constant handling building on Tutorial 1
    const std::string conv2d_rescale_graph =
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
         %45 = OpExtInstImport "TOSA.001000.1"
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
         %22 = OpConstantComposite %_arr_uint_uint_1 %uint_16
         %20 = OpTypeTensorARM %uint %uint_1 %22
         %23 = OpConstantNull %20
     %uint_2 = OpConstant %uint 2
         %25 = OpConstantComposite %_arr_uint_uint_4 %uint_16 %uint_2 %uint_2 %uint_16
         %24 = OpTypeTensorARM %uchar %uint_4 %25
         %27 = OpGraphConstantARM %24 0
         %29 = OpConstantComposite %_arr_uint_uint_1 %uint_4
         %28 = OpTypeTensorARM %uint %uint_1 %29
         %30 = OpConstantNull %28
         %32 = OpConstantComposite %_arr_uint_uint_1 %uint_2
         %31 = OpTypeTensorARM %uint %uint_1 %32
         %33 = OpConstantCompositeReplicateEXT %31 %uint_2
         %34 = OpConstantCompositeReplicateEXT %31 %uint_1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
         %38 = OpConstantComposite %_arr_uint_uint_1 %uint_1
         %37 = OpTypeTensorARM %uchar %uint_1 %38
  %uchar_128 = OpConstant %uchar 128
         %39 = OpConstantComposite %37 %uchar_128
         %41 = OpConstantNull %37
         %43 = OpConstantComposite %_arr_uint_uint_4 %uint_1 %uint_8 %uint_8 %uint_16
         %42 = OpTypeTensorARM %uint %uint_4 %43
       %true = OpConstantTrue %bool
     %uint_3 = OpConstant %uint 3
%uint_1931928506 = OpConstant %uint 1931928506
%uint_1951328493 = OpConstant %uint 1951328493
%uint_1755276611 = OpConstant %uint 1755276611
%uint_1934572170 = OpConstant %uint 1934572170
%uint_1932872072 = OpConstant %uint 1932872072
%uint_1944302913 = OpConstant %uint 1944302913
%uint_1932623941 = OpConstant %uint 1932623941
%uint_1949223780 = OpConstant %uint 1949223780
%uint_1952764318 = OpConstant %uint 1952764318
%uint_1948369062 = OpConstant %uint 1948369062
%uint_1947668025 = OpConstant %uint 1947668025
%uint_1899102491 = OpConstant %uint 1899102491
%uint_1913241564 = OpConstant %uint 1913241564
%uint_1948406415 = OpConstant %uint 1948406415
%uint_1912688338 = OpConstant %uint 1912688338
%uint_1935896203 = OpConstant %uint 1935896203
         %48 = OpConstantComposite %20 %uint_1931928506 %uint_1951328493 %uint_1755276611 %uint_1934572170 %uint_1932872072 %uint_1944302913 %uint_1932623941 %uint_1949223780 %uint_1952764318 %uint_1948369062 %uint_1947668025 %uint_1899102491 %uint_1913241564 %uint_1948406415 %uint_1912688338 %uint_1935896203
         %66 = OpConstantComposite %_arr_uint_uint_1 %uint_16
         %65 = OpTypeTensorARM %uchar %uint_1 %66
   %uchar_42 = OpConstant %uchar 42
         %67 = OpConstantCompositeReplicateEXT %65 %uchar_42
         %70 = OpConstantComposite %_arr_uint_uint_1 %uint_1
         %69 = OpTypeTensorARM %uint %uint_1 %70
         %71 = OpConstantNull %69
   %uchar_26 = OpConstant %uchar 26
         %72 = OpConstantComposite %37 %uchar_26
               OpGraphEntryPointARM %16 "graph_partition_0" %graph_partition_0_arg_0 %graph_partition_0_res_0
         %16 = OpGraphARM %17
         %18 = OpGraphInputARM %2 %uint_0
         %44 = OpExtInst %42 %45 CONV2D %30 %33 %34 %uint_1 %false %18 %27 %23 %39 %41
         %74 = OpExtInst %12 %45 RESCALE %true %uint_3 %true %false %false %44 %48 %67 %71 %72
               OpGraphSetOutputARM %74 %uint_0
               OpGraphEndARM

)"";

    // For the purpose of this sample, we construct the SPIR-V binary from the above SPIR-V disassembly.
    // In production, the code will probably be loaded as SPIR-V binary directly from a .spv file.
    // The SPIR-V code specifically defines a graph type module rather than a compute shader. This will be relevant
    // later in this sample.
    std::vector<uint32_t> spirv_code = sampleutils::spirv_assemble(conv2d_rescale_graph);

    // Create an encoder object
    std::unique_ptr<vgflib::Encoder> encoder = vgflib::CreateEncoder(VK_HEADER_VERSION);

    // Add a graph module
    vgflib::ModuleRef graph_ref =
        encoder->AddModule(vgflib::ModuleType::GRAPH, // Explicitly call out this specific module type as Graph. This
                                                      // needs to match the type of the supplied SPIR-V code.
                           "conv2d_rescale", // Give the module a name. Any name that can help with identification for
                                             // debug/tooling purposes.
                           "main",    // the name of the module entry function, as defined by the OpGraphEntryPointEXT
                                      // SPIR-V instruction in the case of a graph.
                           spirv_code // The SPIR-V binary code for the module
        );

    // Define the input resource
    std::vector<int64_t> input_shape = {1, 16, 16, 16};
    vgflib::ResourceRef input_res_ref =
        encoder->AddInputResource( // this resource is going to be an INPUT to the whole model
            vgflib::ToDescriptorType(VK_DESCRIPTOR_TYPE_TENSOR_ARM), // it will be a vulkan storage tensor
                                                                     // resource type
            vgflib::ToFormatType(VK_FORMAT_R8_SINT),                 // the elements will be int8 type
            input_shape,                                             // 1x16x16x16 input shape
            {}                                                       // empty stides are interpreted as packed linear
        );

    // Define the output resource
    std::vector<int64_t> output_shape = {1, 8, 8, 16};
    vgflib::ResourceRef output_res_ref =
        encoder->AddOutputResource( // this resource is going to be an OUTPUT to the whole model
            vgflib::ToDescriptorType(VK_DESCRIPTOR_TYPE_TENSOR_ARM), // it will be a vulkan storage tensor
                                                                     // resource type
            vgflib::ToFormatType(VK_FORMAT_R8_SINT),                 // the elements will be int8 type
            output_shape,                                            // 1x8x8x16 input shape after a 2x2 maxpool2d op
            {}                                                       // empty strides are interpreted as packed linear
        );

    // Define the binding slots that are ultimately used by DescriptorSets.
    vgflib::BindingSlotRef input_binding_ref = encoder->AddBindingSlot(
        0,            // binding id 0 is the descriptor set binding id expected for the INPUT of the graph above
        input_res_ref // the info (shape, type, etc) about the resource that will be bound at runtime
    );
    vgflib::BindingSlotRef output_binding_ref = encoder->AddBindingSlot(
        1,             // binding id 1 is the Vulkan DescriptorSet binding id for the OUTPUT of the example graph above
        output_res_ref // the info (shape, type, etc) about the resource that will be bound at runtime
    );

    // Input and output are bound to a single descriptor set.
    vgflib::DescriptorSetInfoRef desc_info_ref = encoder->AddDescriptorSetInfo({
        input_binding_ref, // Input binding slot (id=0)
        output_binding_ref // Output binding slot (id=1)
    });

    //===================
    // ** NEW (BEGIN) **
    //===================
    // Define the constants
    // The SPIR-V code above expects 1 graph constant with index 0
    // These can be identified from the OpGraphConstantARM instruction and in this instance, correspond to the following
    // op parameters:
    //      idx 0 = conv2d weight

    // Weights
    std::vector<int64_t> weight_shape = {16, 2, 2, 16};
    vgflib::ResourceRef weight_res_ref = encoder->AddConstantResource( // this resource is going to be a graph constant
        vgflib::ToFormatType(VK_FORMAT_R8_SINT),                       // the weight elements will be int8 type
        weight_shape,                                                  // 16x2x2x16 shape of weights
        {});
    std::vector<int8_t> weight_data(16ULL * 2 * 2 * 16, 0); // This holds the actual bias values
    vgflib::ConstantRef weight_const_ref =
        encoder->AddConstant(weight_res_ref, weight_data.data(), sampleutils::sizeof_vector(weight_data));

    //=================
    // ** NEW (END) **
    //=================

    // Add a segment for the graph.
    encoder->AddSegmentInfo(graph_ref,                       // the graph module
                            "segment_conv2d_rescale_graph2", // the name of the segment for debug/tooling purposes
                            {desc_info_ref},                 // the descriptor sets used by the model
                            {input_binding_ref},  // the input bindings of the segment. Linked resource can be either
                                                  // ResourceCategory INPUT or INTERMEDIATE
                            {output_binding_ref}, // the output bindings of the segment. Linked resource can be
                                                  // either ResourceCategory OUTPUT or INTERMEDIATE
                            {
                                // **NEW** the constants required by the model
                                weight_const_ref, // **NEW** index 0 for weights
                            },
                            {}, // this segment is for a graph module so doesn't require any dispatch shape
                            {}  // this example does not make use of push constants.
    );

    // Define the inputs and outputs for the model. In this simple case, they are the same as the inputs and outputs to
    // the segment.
    encoder->AddModelSequenceInputsOutputs(
        {input_binding_ref},  // the input bindings of the whole model. Should all be of type ResourceCategory::INPUT
        {"input_0"},          // name of the input so that consumers can bind the input data correctly at runtime.
        {output_binding_ref}, // the output binding of the whole model. Should all be of type ResourceCategory::OUTPUT
        {"output_0"}          // name of the output so that consumers can bind the output data correctly at runtime.
    );

    // Mark the encoder as done to finalise any processing before serialization
    encoder->Finish();

    // Write the contents to any valid ostream
    std::string filename = "simple_conv2d_rescale_graph.vgf";
    std::filesystem::path full_path = std::filesystem::temp_directory_path();
    full_path /= filename;

    std::ofstream vgf_file(full_path.c_str(), std::ofstream::binary | std::ofstream::trunc);
    encoder->WriteTo(vgf_file);
    vgf_file.close();

    return full_path.string();
}

} // namespace mlsdk::vgflib::samples
