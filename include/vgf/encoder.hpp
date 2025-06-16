/*
 * SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "types.hpp"

namespace mlsdk::vgflib {

template <typename> class Ref {
  public:
    using RefType = uint32_t;
    const RefType reference;
};

/**
 * \defgroup encoderAPI Encoder API
 * @{
 */

/// \brief Class to store reference to a Module
class ModuleRef : public Ref<ModuleRef> {};

/// \brief Class to store reference to a Resource
class ResourceRef : public Ref<ResourceRef> {};

/// \brief Class to store reference to a Constant
class ConstantRef : public Ref<ConstantRef> {};

/// \brief Class to store reference to a Binding Slot
class BindingSlotRef : public Ref<BindingSlotRef> {};

/// \brief Class to store reference to a Descriptor Set
class DescriptorSetInfoRef : public Ref<DescriptorSetInfoRef> {};

/// \brief Class to store reference to Segment Info
class SegmentInfoRef : public Ref<SegmentInfoRef> {};

/// \brief Class to store reference to a Push Constant Range
class PushConstRangeRef : public Ref<PushConstRangeRef> {};

class Encoder {
  public:
    /// \brief Destructor for the Encoder class
    virtual ~Encoder() = default;

    /// \brief Adds a module, with code, to the VGF
    ///
    /// \param type The type of the module
    /// \param name Unique string name of the module
    /// \param entryPoint Entry point into the shader e.g. "main"
    /// \param code Vector of uint32 representing SPIR-V byte code
    /// \return ModuleRef Type containing information for the added module
    virtual ModuleRef AddModule(ModuleType type, const std::string &name, const std::string &entryPoint,
                                const std::vector<uint32_t> &code = {}) = 0;

    /// \brief Adds a placeholder module to the VGF where the code will be provisioned at load/decode time
    ///
    /// \param type The type of the module
    /// \param name Unique string name of the module
    /// \param entryPoint Entry point into the shader e.g. "main"
    /// \return ModuleRef Type containing information for the added module
    virtual ModuleRef AddPlaceholderModule(ModuleType type, const std::string &name, const std::string &entryPoint) = 0;

    /// \brief Add an INPUT resource to the model resource table
    ///
    /// \param descriptorType The module type of the resource table
    /// \param vkFormat VkFormat of the resource data
    /// \param shape Vector representation of the resource shape. "-1" values represent an unshaped dimension
    /// \param strides Vector representation of the resource stride. An empty strides is assumed packed layout
    /// \return ResourceRef type containing information for the added table entry
    virtual ResourceRef AddInputResource(DescriptorType descriptorType, FormatType vkFormat,
                                         const std::vector<int64_t> &shape, const std::vector<int64_t> &strides) = 0;

    /// \brief Add an OUTPUT resource to the model resource table
    ///
    /// \param descriptorType The module type of the resource table
    /// \param vkFormat VkFormat of the resource data
    /// \param shape Vector representation of the resource shape. "-1" values represent an unshaped dimension
    /// \param strides Vector representation of the resource stride. An empty strides is assumed packed layout
    /// \return ResourceRef type containing information for the added table entry
    virtual ResourceRef AddOutputResource(DescriptorType descriptorType, FormatType vkFormat,
                                          const std::vector<int64_t> &shape, const std::vector<int64_t> &strides) = 0;

    /// \brief Add an INTERMEDIATE resource to the model resource table
    ///
    /// \param descriptorType The module type of the resource table
    /// \param vkFormat VkFormat of the resource data
    /// \param shape Vector representation of the resource shape. "-1" values represent an unshaped dimension
    /// \param strides Vector representation of the resource stride. An empty strides is assumed packed layout
    /// \return ResourceRef type containing information for the added table entry
    virtual ResourceRef AddIntermediateResource(DescriptorType descriptorType, FormatType vkFormat,
                                                const std::vector<int64_t> &shape,
                                                const std::vector<int64_t> &strides) = 0;

    /// \brief Add a CONSTANT tensor to the model resource table
    ///
    /// \param vkFormat VkFormat of the resource data
    /// \param shape Vector representation of the resource shape. "-1" values represent an unshaped dimension
    /// \param strides Vector representation of the resource stride. An empty strides is assumed packed layout
    /// \return ResourceRef type containing information for the added table entry
    virtual ResourceRef AddConstantResource(FormatType vkFormat, const std::vector<int64_t> &shape,
                                            const std::vector<int64_t> &strides) = 0;

    /// \brief Add constant values to a constant resource type in the model resource table
    ///
    /// \param resource Resource reference used in model resource table
    /// \param data Pointer to the memory containing the constant data
    /// \param sizeInBytes Size of the constant data to encode
    /// \param sparsityDimension Dimension on which the constant is sparse
    /// \return ConstantRef type containing information for the added constant
    virtual ConstantRef AddConstant(ResourceRef resource, const void *data, size_t sizeInBytes,
                                    int64_t sparsityDimension = -1) = 0;

    /// \brief Add a binding slot and associate to resource in the model resource table
    ///
    /// \param binding The binding slot to be added
    /// \param resource Ref to resource in model resource table
    /// \return BindingSlotRef type containing information for the added binding slot
    virtual BindingSlotRef AddBindingSlot(uint32_t binding, ResourceRef resource) = 0;

    /// \brief Add descriptor set info for bindings
    ///
    /// \param bindings Vector of binding slot references
    /// \return DescriptorSetInfoRef type containing information for the added descriptor set
    virtual DescriptorSetInfoRef AddDescriptorSetInfo(const std::vector<BindingSlotRef> &bindings = {}) = 0;

    /// \brief Add push constant range to segment info
    ///
    /// \param stageFlags Stage flags describing the shader stages
    /// \param offset Start offset in units of bytes and must be a multiple of 4
    /// \param size Start size in units of bytes and must be a multiple of 4
    /// \return PushConstRangeRef type containing information for the added push constant range
    virtual PushConstRangeRef AddPushConstRange(uint32_t stageFlags, uint32_t offset, uint32_t size) = 0;

    /// \brief Add segment info with approprate references
    ///
    /// \param module Module reference of the added module
    /// \param name Unique string name of the segment
    /// \param descriptors Vector of references to descriptor set info
    /// \param inputs Vector of references to binding slots used as inputs
    /// \param outputs Vector of references to binding slots used as outputs
    /// \param constants Vector of references to segment constants
    /// \param dispatchShape 3-dimensional array of dispatch shape
    /// \param pushConstRanges Vector of references to segment push constant ranges
    /// \return SegmentInfoRef type containing information for the added push constant range
    virtual SegmentInfoRef
    AddSegmentInfo(ModuleRef module, const std::string &name, const std::vector<DescriptorSetInfoRef> &descriptors = {},
                   const std::vector<BindingSlotRef> &inputs = {}, const std::vector<BindingSlotRef> &outputs = {},
                   const std::vector<ConstantRef> &constants = {}, const std::array<uint32_t, 3> &dispatchShape = {},
                   const std::vector<PushConstRangeRef> &pushConstRanges = {}) = 0;

    /// \brief Add the sequence of inputs and outputs to the model
    ///
    /// \param inputs Vector of BindingSlotRefs used as inputs to the model
    /// \param inputNames Vector of std::string containing the names corresponding to inputs.
    ///                   inputNames must be empty or equal inputs.size()
    /// \param outputs Vector of BindingSlotRefs used as outputs to the model
    /// \param outputNames Vector of std::string containing the names corresponding to outputs.
    ///                   outputNames must be empty or equal outputs.size()
    virtual void AddModelSequenceInputsOutputs(const std::vector<BindingSlotRef> &inputs = {},
                                               const std::vector<std::string> &inputNames = {},
                                               const std::vector<BindingSlotRef> &outputs = {},
                                               const std::vector<std::string> &outputNames = {}) = 0;

    /// \brief Inidicate the finishing of VGF file encoding
    virtual void Finish() = 0;

    /// \brief Write the output VGF file
    ///
    /// \param output Output destination of the .vgf file
    /// \return Bool True if write successful
    virtual bool WriteTo(std::ostream &output) = 0;
};

/// \brief Create an Encoder object
///
/// \param vkHeaderVersion Value of VK_HEADER_VERSION as defined in vulkan_core.h as included by the users code.
///                        This is necessary so that loaders of the VGF file know how to interpret "VkFormat" and
///                        "VkDescriptorType" enumeration types.
///                        A value of 0 (NOT RECOMMENDED) indicates that the VGF generating tool does not use the
///                        Vulkan definitions directly.
///
/// \return Encoder object
std::unique_ptr<Encoder> CreateEncoder(uint16_t vkHeaderVersion);

/**@}*/

} // namespace mlsdk::vgflib
