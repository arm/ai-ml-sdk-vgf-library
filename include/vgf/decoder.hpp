/*
 * SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include "types.hpp"

namespace mlsdk::vgflib {

template <typename T> class DataView {
  public:
    constexpr DataView(const T *ptr, size_t size) noexcept : _ptr(ptr), _size(size) {}
    constexpr DataView() noexcept = default;

    constexpr size_t size() const noexcept { return _size; }
    constexpr T const &operator[](uint32_t i) const noexcept { return _ptr[i]; };

    constexpr auto begin() const noexcept { return _ptr; }
    constexpr auto end() const noexcept { return _ptr + _size; }
    constexpr bool empty() const noexcept { return begin() == end(); }

    constexpr bool operator==(const DataView<T> &other) const noexcept {
        if (_size != other._size) {
            return false;
        }

        if (!_ptr && !other._ptr) {
            return true;
        }

        if (!_ptr || !other._ptr) {
            return false;
        }

        return std::memcmp(_ptr, other._ptr, _size) == 0;
    }

  private:
    const T *_ptr = nullptr;
    size_t _size = 0;
};

/**
 * \defgroup VGFDecoderAPI Decoder API
 * @{
 */

class HeaderDecoder {
  public:
    virtual ~HeaderDecoder() = default;
    /**
     * @brief Checks the header magic value of the VGF file
     */
    virtual bool IsValid() const = 0;

    /**
     * @brief Checks if VGF version is the latest
     */
    virtual bool IsLatestVersion() const = 0;

    /**
     * @brief Get the value of VK_HEADER_VERSION used by Encoding tool
     */

    virtual uint16_t GetEncoderVulkanHeadersVersion() const = 0;

    /**
     * @brief Checks the VGF version
     */
    virtual bool CheckVersion() const = 0;

    /**
     * @brief Returns the Major version value
     */
    virtual uint8_t GetMajor() const = 0;

    /**
     * @brief Returns the Minor version value
     */
    virtual uint8_t GetMinor() const = 0;

    /**
     * @brief Returns the Patch version value
     */
    virtual uint8_t GetPatch() const = 0;

    /**
     * @brief Returns the size of the Module Table in memory
     */
    virtual uint64_t GetModuleTableSize() const = 0;

    /**
     * @brief Returns the relative location of the Module Table in memory
     */
    virtual uint64_t GetModuleTableOffset() const = 0;

    /**
     * @brief Returns the relative location of the Model Sequence Table in memory
     */
    virtual uint64_t GetModelSequenceTableOffset() const = 0;

    /**
     * @brief Returns the size of the Model Sequence Table in memory
     */
    virtual uint64_t GetModelSequenceTableSize() const = 0;

    /**
     * @brief Returns the relative location of the Model Resource Table in memory
     */
    virtual uint64_t GetModelResourceTableOffset() const = 0;

    /**
     * @brief Returns the size of the Model Resource Table in memory
     */
    virtual uint64_t GetModelResourceTableSize() const = 0;

    /**
     * @brief Returns the size of the Constant section in memory
     */
    virtual uint64_t GetConstantsSize() const = 0;

    /**
     * @brief Returns the relative location of the Constant section in memory
     */
    virtual uint64_t GetConstantsOffset() const = 0;
};

/**
 * @brief Returns the size of Header section in memory'
 *
 */
size_t HeaderSize();

/**
 * @brief Returns the size of Header decoder in memory'
 *
 */
size_t HeaderDecoderSize();

/**
 * @brief Constructs a Header decoder
 *
 * @param data
 */
std::unique_ptr<HeaderDecoder> CreateHeaderDecoder(const void *data);

/**
 * @brief Constructs a Header decoder in-place using pre-allocated memory
 *
 * @param data
 * @param decoderMem
 */
HeaderDecoder *CreateHeaderDecoderInPlace(const void *data, void *decoderMem);

// ModuleTableDecoder
class ModuleTableDecoder {
  public:
    virtual ~ModuleTableDecoder() = default;
    /**
     * @brief Retrieves the number of entries in the Module Table
     */
    virtual size_t size() const = 0;

    /**
     * @brief Retrieves the moduleType of module 'idx'
     *
     * @param idx
     */
    virtual ModuleType getModuleType(uint32_t idx) const = 0;

    /**
     * @brief Checks if the table entry 'idx' has SPIR-V code
     *
     * @param idx
     */
    virtual bool hasSPIRV(uint32_t idx) const = 0;

    /**
     * @brief Retrieves the module name of module 'idx'
     *
     * @param idx
     */
    virtual std::string_view getModuleName(uint32_t idx) const = 0;

    /**
     * @brief Returns the entry point of module 'idx'
     *
     * @param idx
     */
    virtual std::string_view getModuleEntryPoint(uint32_t idx) const = 0;

    /**
     * @brief Retrieves the SPIR-V code of the module 'idx' in the Module Table
     *
     * @param idx
     */
    virtual DataView<uint32_t> getModuleCode(uint32_t idx) const = 0;
};

/**
 * @brief Returns the size of Module Table decoder in memory'
 *
 */
size_t ModuleTableDecoderSize();

/**
 * @brief Returns true if input points to a valid Module Table
 *
 * @param data
 * @param size Max count of read bytes
 */
bool VerifyModuleTable(const void *data, uint64_t size);

/**
 * @brief Constructs a Module Table decoder
 *
 * @param data
 */
std::unique_ptr<ModuleTableDecoder> CreateModuleTableDecoder(const void *data);

/**
 * @brief Constructs a Module Table decoder in-place using pre-allocated memory
 *
 * @param data
 * @param decoderMem
 */
ModuleTableDecoder *CreateModuleTableDecoderInPlace(const void *data, void *decoderMem);

// ModelResourceTableDecoder
class ModelResourceTableDecoder {
  public:
    virtual ~ModelResourceTableDecoder() = default;
    /**
     * @brief Returns the number of entries in the Model Resource Table
     *
     */
    virtual size_t size() const = 0;

    /**
     * @brief Returns the DescriptorType of the MRT entry 'id'
     *
     * @param id
     */
    virtual std::optional<DescriptorType> getDescriptorType(uint32_t id) const = 0;

    /**
     * @brief Returns the VkFormat of the MRT entry 'id'
     *
     * @param id
     */
    virtual FormatType getVkFormat(uint32_t id) const = 0;

    /**
     * @brief Returns the ResourceCategory of the MRT entry 'id'
     *
     * @param id
     */
    virtual ResourceCategory getCategory(uint32_t id) const = 0;

    /**
     * @brief Returns the tensor shape of the MRT entry 'id'. A "-1" value represents an unshaped dimension
     *
     * @param id
     */
    virtual DataView<int64_t> getTensorShape(uint32_t id) const = 0;

    /**
     * @brief Returns the tensor stride of the MRT entry 'id'
     *
     * @param id
     */
    virtual DataView<int64_t> getTensorStride(uint32_t id) const = 0;
};

/**
 * @brief Returns the size of Module Resource Table decoder in memory'
 *
 */
size_t ModelResourceTableDecoderSize();

/**
 * @brief Returns true if input points to a valid Model Resource Table
 *
 * @param data
 * @param size Max count of read bytes
 */
bool VerifyModelResourceTable(const void *data, uint64_t size);

/**
 * @brief Constructs a Model Resource Table decoder
 *
 * @param data
 */
std::unique_ptr<ModelResourceTableDecoder> CreateModelResourceTableDecoder(const void *data);

/**
 * @brief Constructs a Model Resource Table decoder in-place using pre-allocated memory
 *
 * @param data
 * @param decoderMem
 */
ModelResourceTableDecoder *CreateModelResourceTableDecoderInPlace(const void *data, void *decoderMem);

// ConstantDecoder
class ConstantDecoder {
  public:
    virtual ~ConstantDecoder() = default;
    /**
     * @brief Returns the number of constants in the Constant section
     *
     */
    virtual size_t size() const = 0;

    /**
     * @brief Returns the Model Resource Table index of constant 'idx'
     *
     * @param idx Index of the constant
     */
    virtual uint32_t getConstantMrtIndex(uint32_t idx) const = 0;

    /**
     * @brief Returns true if constant is 2:4 sparse on one dimension.
     *
     * @param idx Index of the constant
     */
    virtual bool isSparseConstant(uint32_t idx) const = 0;

    /**
     * @brief Returns the dimension on which the constant is sparse.
     *
     * @param idx Index of the constant
     */
    virtual int64_t getConstantSparsityDimension(uint32_t idx) const = 0;

    /**
     * @brief Returns the constant at location 'idx' in the Constant section
     *
     * @param idx Index of the constant
     */
    virtual DataView<uint8_t> getConstant(uint32_t idx) const = 0;
};

/**
 * @brief Returns the size of Constant decoder in memory'
 *
 */
size_t ConstantDecoderSize();

/**
 * @brief Returns true if input points to a valid Constant section
 *
 * @param data Pointer to the Constants section data
 * @param size Max count of read bytes
 */
bool VerifyConstant(const void *data, uint64_t size);

/**
 * @brief Constructs a Constant section decoder
 *
 * @param data Pointer to the Constants section data
 */
std::unique_ptr<ConstantDecoder> CreateConstantDecoder(const void *data);

/**
 * @brief Constructs a Constant section decoder in-place using preallocated memory
 *
 * @param data Pointer to the Constants section data
 * @param decoderMem memory in which to create the decoder
 */
ConstantDecoder *CreateConstantDecoderInPlace(const void *data, void *decoderMem);

// Binding Slot Array Handle
struct BindingSlotArrayHandle_s {};
using BindingSlotArrayHandle = const BindingSlotArrayHandle_s *;

// Binding name Array Handle
struct NameArrayHandle_s {};
using NameArrayHandle = const NameArrayHandle_s *;

// Push Constant Range Handle
struct PushConstantRangeHandle_s {};
using PushConstantRangeHandle = const PushConstantRangeHandle_s *;

// Model Sequence Table Decoder
class ModelSequenceTableDecoder {
  public:
    virtual ~ModelSequenceTableDecoder() = default;
    /**
     * @brief Returns the number of segments
     *
     */
    virtual size_t modelSequenceTableSize() const = 0;

    /**
     * @brief Returns the number of descriptor set infos for segment 'segmentIdx' in the Model Sequence Table
     *
     * @param segmentIdx
     */
    virtual size_t getSegmentDescriptorSetInfosSize(uint32_t segmentIdx) const = 0;

    /**
     * @brief Returns the constants that are linked to segment 'segmentIdx' in the Model Sequence Table
     *
     * @param segmentIdx
     */
    virtual DataView<uint32_t> getSegmentConstantIndexes(uint32_t segmentIdx) const = 0;

    /**
     * @brief Returns the ModuleType of segment 'segmentIdx'
     *
     * @param segmentIdx
     */
    virtual ModuleType getSegmentType(uint32_t segmentIdx) const = 0;

    /**
     * @brief Returns the name of segment 'segmentIdx'
     *
     * @param segmentIdx
     */
    virtual std::string_view getSegmentName(uint32_t segmentIdx) const = 0;

    /**
     * @brief Returns the name of segment 'segmentIdx'
     *
     * @param segmentIdx
     */
    virtual uint32_t getSegmentModuleIndex(uint32_t segmentIdx) const = 0;

    /**
     * @brief Returns the dispatch shape for segment 'segIdx'
     *
     * @param segmentIdx
     */
    virtual DataView<uint32_t> getSegmentDispatchShape(uint32_t segmentIdx) const = 0;

    /**
     * @brief Retrieves the 'BindingSlotArrayHandle' corresponding to segment 'segmentIdx' descriptor 'descIdx'
     *
     * @param segmentIdx
     * @param descIdx
     * @returns Handle to the binding slot array
     */
    virtual BindingSlotArrayHandle getDescriptorBindingSlotsHandle(uint32_t segmentIdx, uint32_t descIdx) const = 0;

    /**
     * @brief Retrieves the inputs 'BindingSlotArrayHandle' corresponding to segment 'segmentIdx'
     *
     * @param segmentIdx
     * @returns Handle to the binding slot array
     */
    virtual BindingSlotArrayHandle getSegmentInputBindingSlotsHandle(uint32_t segmentIdx) const = 0;

    /**
     * @brief Retrieves the outputs 'BindingSlotArrayHandle' corresponding to segment 'segmentIdx'
     *
     * @param segmentIdx
     * @returns Handle to the binding slot array
     */
    virtual BindingSlotArrayHandle getSegmentOutputBindingSlotsHandle(uint32_t segmentIdx) const = 0;

    /**
     * @brief Retrieves the inputs 'BindingSlotArrayHandle' for the model sequence
     *
     * @returns Handle to the binding slot array
     */
    virtual BindingSlotArrayHandle getModelSequenceInputBindingSlotsHandle() const = 0;

    /**
     * @brief Retrieves the outputs 'BindingSlotArrayHandle' for the model sequence
     *
     * @returns Handle to the binding slot array
     */
    virtual BindingSlotArrayHandle getModelSequenceOutputBindingSlotsHandle() const = 0;

    /**
     * @brief Retrieves the 'NameArrayHandle' for the inputs of the model
     *
     * @returns Handle to the name array
     */
    virtual NameArrayHandle getModelSequenceInputNamesHandle() const = 0;

    /**
     * @brief Retrieves the 'NameArrayHandle' for the outputs of the model
     *
     * @returns Handle to the name array
     */
    virtual NameArrayHandle getModelSequenceOutputNamesHandle() const = 0;

    /**
     * @brief Retrieves the number of names
     *
     * @param handle Opaque handle to an array of names
     * @returns The number of names in the array referenced by handle
     */
    virtual size_t getNamesSize(NameArrayHandle handle) const = 0;

    /**
     * @brief Retrieves the name at a given index in the array
     *
     * @param handle Opaque handle to an array of names
     * @param nameIdx Index of the name in the array to fetch
     * @returns name
     */
    virtual std::string_view getName(NameArrayHandle handle, uint32_t nameIdx) const = 0;

    /**
     * @brief Retrieves the number of bindings
     *
     * @param handle Opaque handle to an array of BindSlots
     * @returns The number of BindingSlots in the array referenced by handle
     */
    virtual size_t getBindingsSize(BindingSlotArrayHandle handle) const = 0;

    /**
     * @brief Retrieves the bindings at slot 'slotIdx'
     *
     * @param handle Opaque handle to an array of BindSlots
     * @param slotIdx index of the slot in the the array of bindingslots referenced by the handle
     * @returns The binding id of the selected BindingSlot
     */
    virtual uint32_t getBindingSlotBinding(BindingSlotArrayHandle handle, uint32_t slotIdx) const = 0;

    /**
     * @brief Retrieves the MRT index which is associated to the binding slot 'slotIdx'
     *
     * @param handle Opaque handle to an array of BindSlots
     * @param slotIdx index of the slot in the the array of bindingslots referenced by the handle
     * @returns The index of the entry in Model Resource Table referenced by this binding slot
     */
    virtual uint32_t getBindingSlotMrtIndex(BindingSlotArrayHandle handle, uint32_t slotIdx) const = 0;

    /**
     * @brief Retrieves the push constant ranges corresponding to segment 'segmentIdx' and stores them in target
     *
     * @param segmentIdx Index of the segment to query
     * @returns The handle to the push constants range for the segment
     */
    virtual PushConstantRangeHandle getSegmentPushConstRange(uint32_t segmentIdx) const = 0;

    /**
     * @brief Returns the number of push constant ranges
     *
     * @param handle The handle to the push constants range
     */
    virtual size_t getPushConstRangesSize(PushConstantRangeHandle handle) const = 0;

    /**
     * @brief Returns the stage flags of push constant range 'rangeIdx'
     *
     * @param handle The handle to the push constants range
     * @param rangeIdx Index of the specific range to query
     */
    virtual uint32_t getPushConstRangeStageFlags(PushConstantRangeHandle handle, uint32_t rangeIdx) const = 0;

    /**
     * @brief Returns the offset of push constant range 'rangeIdx'
     *
     * @param handle The handle to the push constants range
     * @param rangeIdx Index of the specific range to query
     */
    virtual uint32_t getPushConstRangeOffset(PushConstantRangeHandle handle, uint32_t rangeIdx) const = 0;

    /**
     * @brief Returns the size of push constant range 'rangeIdx'
     *
     * @param handle The handle to the push constants range
     * @param rangeIdx Index of the specific range to query
     */
    virtual uint32_t getPushConstRangeSize(PushConstantRangeHandle handle, uint32_t rangeIdx) const = 0;
};

/**
 * @brief Returns true if input points to a valid Model Sequence Table
 *
 * @param data
 * @param size Max count of read bytes
 */
bool VerifyModelSequenceTable(const void *data, uint64_t size);

/**
 * @brief Constructs a Model Sequence Table decoder
 *
 * @param data
 */
std::unique_ptr<ModelSequenceTableDecoder> CreateModelSequenceTableDecoder(const void *data);

/**
 * @brief Constructs a Model Sequence Table decoder in-place using pre-allocated memory
 *
 * @param data
 * @param decoderMem
 */
ModelSequenceTableDecoder *CreateModelSequenceTableDecoderInPlace(const void *data, void *decoderMem);

/**@}*/
} // namespace mlsdk::vgflib
