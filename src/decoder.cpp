/*
 * SPDX-FileCopyrightText: Copyright 2023-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf/decoder.hpp"

#include "constant.hpp"
#include "header.hpp"
#include "internal_logging.hpp"
#include "internal_types.hpp"
#include "vgf_generated.h"

#include <cassert>
#include <optional>
#include <tuple>

namespace mlsdk::vgflib {
namespace {

template <typename T> inline T ReadBytesAs(const void *const baseAddr, size_t offset) {
    const auto *bytes = static_cast<const uint8_t *>(baseAddr) + offset;
    return *reinterpret_cast<const T *>(bytes);
}

ModuleType fromVGF(VGF::ModuleType type) {
    switch (type) {
    case VGF::ModuleType::ModuleType_COMPUTE:
        return ModuleType::COMPUTE;
    case VGF::ModuleType::ModuleType_GRAPH:
        return ModuleType::GRAPH;
    default:
        assert(false && "unknown Moduletype");
        return ModuleType::COMPUTE;
    }
}

ResourceCategory fromVGF(VGF::ResourceCategory category) {
    switch (category) {
    case VGF::ResourceCategory::ResourceCategory_INPUT:
        return ResourceCategory::INPUT;
    case VGF::ResourceCategory::ResourceCategory_OUTPUT:
        return ResourceCategory::OUTPUT;
    case VGF::ResourceCategory::ResourceCategory_INTERMEDIATE:
        return ResourceCategory::INTERMEDIATE;
    case VGF::ResourceCategory::ResourceCategory_CONSTANT:
        return ResourceCategory::CONSTANT;
    default:
        assert(false && "unknown ResourceCategory");
        return ResourceCategory::INPUT;
    }
}

constexpr FourCCValue OldMagicAsFourCC() {
    auto v = HEADER_MAGIC_VALUE_OLD;

    auto ToChar = [](const uint32_t val, uint32_t shift) {
        uint32_t a = (val >> shift) & 255;
        return static_cast<char>(a);
    };

    return FourCC(ToChar(v, 0), ToChar(v, 8), ToChar(v, 16), ToChar(v, 24));
}

template <class T> bool VerifyImpl(const void *data, const uint64_t size) {
    const auto *obj = flatbuffers::GetRoot<const T>(data);
    flatbuffers::Verifier verifier(static_cast<const uint8_t *>(data), size);
    return obj->Verify(verifier);
}

const char *getConstantSectionVersion(const void *data) {
    return static_cast<const char *>(data) + CONSTANT_SECTION_VERSION_OFFSET;
}

} // namespace

// Header Decoder
class HeaderDecoderImpl : public HeaderDecoder {
  public:
    explicit HeaderDecoderImpl(const void *const data) : _header(static_cast<const Header *>(data)) {}

    [[nodiscard]] bool IsLatestVersion() const override {
        return IsValid() && GetMajor() == HEADER_MAJOR_VERSION_VALUE && GetMinor() == HEADER_MINOR_VERSION_VALUE &&
               GetPatch() == HEADER_PATCH_VERSION_VALUE;
    }

    [[nodiscard]] bool IsValid() const override {
        return _header->magic == HEADER_MAGIC_VALUE || _header->magic == OldMagicAsFourCC();
    }

    [[nodiscard]] uint16_t GetEncoderVulkanHeadersVersion() const override { return _header->vkHeaderVersion; }

    [[nodiscard]] bool CheckVersion() const override {
        return IsValid() && GetMajor() == HEADER_MAJOR_VERSION_VALUE && GetMinor() <= HEADER_MINOR_VERSION_VALUE;
    }

    [[nodiscard]] uint8_t GetMajor() const override { return _header->version.major; }
    [[nodiscard]] uint8_t GetMinor() const override { return _header->version.minor; }
    [[nodiscard]] uint8_t GetPatch() const override { return _header->version.patch; }

    [[nodiscard]] uint64_t GetModuleTableSize() const override {
        return ReadBytesAs<uint64_t>(_header, HEADER_MODULE_SECTION_SIZE_OFFSET);
    }
    [[nodiscard]] uint64_t GetModuleTableOffset() const override {
        return ReadBytesAs<uint64_t>(_header, HEADER_MODULE_SECTION_OFFSET_OFFSET);
    }
    [[nodiscard]] uint64_t GetModelSequenceTableSize() const override {
        return ReadBytesAs<uint64_t>(_header, HEADER_MODEL_SEQUENCE_SECTION_SIZE_OFFSET);
    }
    [[nodiscard]] uint64_t GetModelSequenceTableOffset() const override {
        return ReadBytesAs<uint64_t>(_header, HEADER_MODEL_SEQUENCE_SECTION_OFFSET_OFFSET);
    }
    [[nodiscard]] uint64_t GetModelResourceTableSize() const override {
        return ReadBytesAs<uint64_t>(_header, HEADER_MODEL_RESOURCE_SECTION_SIZE_OFFSET);
    }
    [[nodiscard]] uint64_t GetModelResourceTableOffset() const override {
        return ReadBytesAs<uint64_t>(_header, HEADER_MODEL_RESOURCE_SECTION_OFFSET_OFFSET);
    }
    [[nodiscard]] uint64_t GetConstantsSize() const override {
        return ReadBytesAs<uint64_t>(_header, HEADER_CONSTANT_SECTION_SIZE_OFFSET);
    }
    [[nodiscard]] uint64_t GetConstantsOffset() const override {
        return ReadBytesAs<uint64_t>(_header, HEADER_CONSTANT_SECTION_OFFSET_OFFSET);
    }

  private:
    const Header *const _header;
};

size_t HeaderSize() { return HEADER_HEADER_SIZE_VALUE; }
size_t HeaderDecoderSize() { return sizeof(HeaderDecoderImpl); }

std::unique_ptr<HeaderDecoder> CreateHeaderDecoder(const void *const data, const uint64_t size) {
    assert(data != nullptr && "data is null");

    if (size < HeaderSize()) {
        logging::error("Header size is smaller than expected");
        return nullptr;
    }
    return std::make_unique<HeaderDecoderImpl>(data);
}

HeaderDecoder *CreateHeaderDecoderInPlace(const void *const data, const uint64_t size, void *decoderMem) {
    assert(data != nullptr && "data is null");
    assert(decoderMem != nullptr && "decoderMem is null");
    if (size < HeaderSize()) {
        logging::error("Header size is smaller than expected");
        return nullptr;
    }
    return new (decoderMem) HeaderDecoderImpl(data);
}

// Module Table decoder
class ModuleTableDecoderImpl : public ModuleTableDecoder {
  public:
    explicit ModuleTableDecoderImpl(const void *const data, uint64_t size)
        : _moduleTable(flatbuffers::GetRoot<const VGF::ModuleTable>(data)) {
        (void)size;
    }

    [[nodiscard]] size_t size() const override {
        const auto *modules = _moduleTable->modules();
        return modules == nullptr ? 0 : modules->size();
    }

    [[nodiscard]] ModuleType getModuleType(uint32_t idx) const override { return fromVGF(getModuleAt(idx)->type()); }

    [[nodiscard]] std::string_view getModuleName(uint32_t idx) const override {
        return std::string_view{getModuleAt(idx)->name()->c_str()};
    }

    [[nodiscard]] bool hasSPIRV(uint32_t idx) const override {
        return VGF::ModuleCode_SPIRV == getModuleAt(idx)->code_type();
    }

    [[nodiscard]] std::string_view getModuleEntryPoint(uint32_t idx) const override {
        return std::string_view{getModuleAt(idx)->entry_point()->c_str()};
    }

    [[nodiscard]] DataView<uint32_t> getModuleCode(uint32_t idx) const override {
        const VGF::SPIRV *spirv = getModuleAt(idx)->code_as_SPIRV();
        if (spirv && spirv->words()) {
            return {spirv->words()->data(), spirv->words()->size()};
        }
        return {};
    }

  private:
    [[nodiscard]] const VGF::Module *getModuleAt(uint32_t idx) const {
        const auto *modules = _moduleTable->modules();
        assert(modules && "no modules found");
        return modules->Get(idx);
    }

    const VGF::ModuleTable *_moduleTable;
};

size_t ModuleTableDecoderSize() { return sizeof(ModuleTableDecoderImpl); }

bool VerifyModuleTable(const void *data, const uint64_t size) {
    assert(data != nullptr && "data is null");
    return VerifyImpl<VGF::ModuleTable>(data, size);
}

std::unique_ptr<ModuleTableDecoder> CreateModuleTableDecoder(const void *const data, const uint64_t size) {
    assert(data != nullptr && "data is null");
    return std::make_unique<ModuleTableDecoderImpl>(data, size);
}

ModuleTableDecoder *CreateModuleTableDecoderInPlace(const void *const data, const uint64_t size, void *decoderMem) {
    assert(data != nullptr && "data is null");
    assert(decoderMem != nullptr && "decoderMem is null");
    return new (decoderMem) ModuleTableDecoderImpl(data, size);
}

namespace {
const flatbuffers::Vector<flatbuffers::Offset<VGF::BindingSlot>> *FromHandle(BindingSlotArrayHandle handle) {
    return reinterpret_cast<const flatbuffers::Vector<flatbuffers::Offset<VGF::BindingSlot>> *>(handle);
}

BindingSlotArrayHandle ToHandle(const flatbuffers::Vector<flatbuffers::Offset<VGF::BindingSlot>> *ptr) {
    return reinterpret_cast<BindingSlotArrayHandle>(ptr);
}

const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *FromHandle(NameArrayHandle handle) {
    return reinterpret_cast<const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *>(handle);
}

NameArrayHandle ToHandle(const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *ptr) {
    return reinterpret_cast<NameArrayHandle>(ptr);
}

const flatbuffers::Vector<flatbuffers::Offset<VGF::PushConstantRange>> *FromHandle(PushConstantRangeHandle handle) {
    return reinterpret_cast<const flatbuffers::Vector<flatbuffers::Offset<VGF::PushConstantRange>> *>(handle);
}

PushConstantRangeHandle ToHandle(const flatbuffers::Vector<flatbuffers::Offset<VGF::PushConstantRange>> *ptr) {
    return reinterpret_cast<PushConstantRangeHandle>(ptr);
}

} // namespace

// Model Sequence Table Decoder
class ModelSequenceTableDecoderImpl : public ModelSequenceTableDecoder {
  public:
    explicit ModelSequenceTableDecoderImpl(const void *const data, uint64_t size)
        : _modelSequenceTable(flatbuffers::GetRoot<const VGF::ModelSequenceTable>(data)) {
        (void)size;
    }

    [[nodiscard]] size_t modelSequenceTableSize() const override {
        const auto *segments = _modelSequenceTable->segments();
        return segments == nullptr ? 0 : segments->size();
    }

    [[nodiscard]] size_t getSegmentDescriptorSetInfosSize(uint32_t segmentIdx) const override {
        const auto *descriptors = getSegmentAt(segmentIdx)->set_infos();
        return descriptors == nullptr ? 0 : descriptors->size();
    }

    [[nodiscard]] DataView<uint32_t> getSegmentConstantIndexes(uint32_t segmentIdx) const override {
        const auto *constants = getSegmentAt(segmentIdx)->constants();
        if (constants == nullptr) {
            return {};
        }
        return {constants->data(), constants->size()};
    }

    [[nodiscard]] DataView<uint32_t> getSegmentDispatchShape(uint32_t segmentIdx) const override {
        const auto *dispatchShape = getSegmentAt(segmentIdx)->dispatch_shape();
        if (dispatchShape == nullptr) {
            return {};
        }
        return {dispatchShape->data(), dispatchShape->size()};
    }

    [[nodiscard]] BindingSlotArrayHandle getDescriptorBindingSlotsHandle(uint32_t segmentIdx,
                                                                         uint32_t descIdx) const override {
        return ToHandle(getDescriptorAt(getSegmentAt(segmentIdx), descIdx)->bindings());
    }

    [[nodiscard]] BindingSlotArrayHandle getSegmentInputBindingSlotsHandle(uint32_t segmentIdx) const override {
        return ToHandle(getSegmentAt(segmentIdx)->inputs());
    }

    [[nodiscard]] BindingSlotArrayHandle getSegmentOutputBindingSlotsHandle(uint32_t segmentIdx) const override {
        return ToHandle(getSegmentAt(segmentIdx)->outputs());
    }

    [[nodiscard]] BindingSlotArrayHandle getModelSequenceInputBindingSlotsHandle() const override {
        return ToHandle(_modelSequenceTable->inputs());
    }

    [[nodiscard]] BindingSlotArrayHandle getModelSequenceOutputBindingSlotsHandle() const override {
        return ToHandle(_modelSequenceTable->outputs());
    }

    [[nodiscard]] size_t getBindingsSize(BindingSlotArrayHandle handle) const override {
        return FromHandle(handle)->size();
    }

    uint32_t getBindingSlotBinding(BindingSlotArrayHandle handle, uint32_t slotIdx) const override {
        return FromHandle(handle)->Get(slotIdx)->binding();
    }

    uint32_t getBindingSlotMrtIndex(BindingSlotArrayHandle handle, uint32_t slotIdx) const override {
        return FromHandle(handle)->Get(slotIdx)->mrt_index();
    }

    PushConstantRangeHandle getSegmentPushConstRange(uint32_t segmentIdx) const override {
        return ToHandle(getSegmentAt(segmentIdx)->push_constant_ranges());
    }

    [[nodiscard]] NameArrayHandle getModelSequenceInputNamesHandle() const override {
        return ToHandle(_modelSequenceTable->input_names());
    }

    [[nodiscard]] NameArrayHandle getModelSequenceOutputNamesHandle() const override {
        return ToHandle(_modelSequenceTable->output_names());
    }

    [[nodiscard]] size_t getNamesSize(NameArrayHandle handle) const override {
        if (handle == nullptr) {
            return 0;
        }
        return FromHandle(handle)->size();
    }

    [[nodiscard]] std::string_view getName(NameArrayHandle handle, uint32_t nameIdx) const override {
        return std::string_view{FromHandle(handle)->Get(nameIdx)->c_str()};
    }

    [[nodiscard]] size_t getPushConstRangesSize(PushConstantRangeHandle handle) const override {
        return FromHandle(handle)->size();
    }

    [[nodiscard]] uint32_t getPushConstRangeStageFlags(PushConstantRangeHandle handle,
                                                       uint32_t rangeIdx) const override {
        return FromHandle(handle)->Get(rangeIdx)->stage_flags();
    }

    [[nodiscard]] uint32_t getPushConstRangeOffset(PushConstantRangeHandle handle, uint32_t rangeIdx) const override {
        return FromHandle(handle)->Get(rangeIdx)->offset();
    }

    [[nodiscard]] uint32_t getPushConstRangeSize(PushConstantRangeHandle handle, uint32_t rangeIdx) const override {
        return FromHandle(handle)->Get(rangeIdx)->size();
    }

    [[nodiscard]] ModuleType getSegmentType(uint32_t segmentIdx) const override {
        return fromVGF(getSegmentAt(segmentIdx)->type());
    }

    [[nodiscard]] std::string_view getSegmentName(uint32_t segmentIdx) const override {
        return std::string_view{getSegmentAt(segmentIdx)->name()->c_str()};
    }

    [[nodiscard]] uint32_t getSegmentModuleIndex(uint32_t segmentIdx) const override {
        return getSegmentAt(segmentIdx)->module_index();
    }

  private:
    [[nodiscard]] const VGF::SegmentInfo *getSegmentAt(uint32_t segmentIdx) const {
        const auto *segments = _modelSequenceTable->segments();
        assert(segments && "no segment found at given index");
        return segments->Get(segmentIdx);
    }

    static const VGF::DescriptorSetInfo *getDescriptorAt(const VGF::SegmentInfo *segment, uint32_t segmentIdx) {
        const auto *descriptors = segment->set_infos();
        assert(descriptors && "no descriptor found at given index");
        return descriptors->Get(segmentIdx);
    }

    const VGF::ModelSequenceTable *_modelSequenceTable;
};

bool VerifyModelSequenceTable(const void *data, const uint64_t size) {
    assert(data != nullptr && "data is null");
    return VerifyImpl<VGF::ModelSequenceTable>(data, size);
}

std::unique_ptr<ModelSequenceTableDecoder> CreateModelSequenceTableDecoder(const void *const data,
                                                                           const uint64_t size) {
    assert(data != nullptr && "data is null");
    return std::make_unique<ModelSequenceTableDecoderImpl>(data, size);
}

ModelSequenceTableDecoder *CreateModelSequenceTableDecoderInPlace(const void *const data, const uint64_t size,
                                                                  void *decoderMem) {
    assert(data != nullptr && "data is null");
    assert(decoderMem != nullptr && "decoderMem is null");
    return new (decoderMem) ModelSequenceTableDecoderImpl(data, size);
}

// Model Resource Table Decoder
class ModelResourceTableDecoderImpl : public ModelResourceTableDecoder {
  public:
    explicit ModelResourceTableDecoderImpl(const void *const data, uint64_t size)
        : _modelRecTable(flatbuffers::GetRoot<const VGF::ModelResourceTable>(data)) {
        (void)size;
    }

    [[nodiscard]] size_t size() const override {
        const auto *entries = _modelRecTable->mrt_entry();
        return entries == nullptr ? 0 : entries->size();
    }

    [[nodiscard]] std::optional<DescriptorType> getDescriptorType(uint32_t id) const override {
        const auto *entry = getEntryAt(id);

        auto decDescType = static_cast<EncodedDescriptorType>(entry->vk_descriptor_type());
        if (decDescType == NullOptDescriptorType()) {
            return std::nullopt;
        }
        return static_cast<DescriptorType>(decDescType);
    }

    [[nodiscard]] FormatType getVkFormat(uint32_t id) const override {
        const auto *entry = getEntryAt(id);
        return static_cast<FormatType>(entry->vk_format());
    }

    [[nodiscard]] ResourceCategory getCategory(uint32_t id) const override {
        const auto *entry = getEntryAt(id);
        return fromVGF(entry->category());
    }

    [[nodiscard]] DataView<int64_t> getTensorShape(uint32_t id) const override {
        const auto *entry = getEntryAt(id);
        const auto *shape = entry->description()->shape();
        if (shape == nullptr) {
            return {};
        }
        return {shape->data(), shape->size()};
    }

    [[nodiscard]] DataView<int64_t> getTensorStride(uint32_t id) const override {
        const auto *entry = getEntryAt(id);
        const auto *strides = entry->description()->strides();
        if (strides == nullptr) {
            return {};
        }
        return {strides->data(), strides->size()};
    }

  private:
    [[nodiscard]] const VGF::ModelResourceTableEntry *getEntryAt(uint32_t index) const {
        const auto *entryTable = _modelRecTable->mrt_entry();
        assert(entryTable && "no entryTable found");
        return entryTable->Get(index);
    }
    const VGF::ModelResourceTable *_modelRecTable;
};

size_t ModelResourceTableDecoderSize() { return sizeof(ModelResourceTableDecoderImpl); }

bool VerifyModelResourceTable(const void *data, const uint64_t size) {
    assert(data != nullptr && "data is null");
    return VerifyImpl<VGF::ModelResourceTable>(data, size);
}

std::unique_ptr<ModelResourceTableDecoder> CreateModelResourceTableDecoder(const void *const data,
                                                                           const uint64_t size) {
    assert(data != nullptr && "data is null");
    return std::make_unique<ModelResourceTableDecoderImpl>(data, size);
}

ModelResourceTableDecoder *CreateModelResourceTableDecoderInPlace(const void *const data, const uint64_t size,
                                                                  void *decoderMem) {
    assert(data != nullptr && "data is null");
    assert(decoderMem != nullptr && "decoderMem is null");
    return new (decoderMem) ModelResourceTableDecoderImpl(data, size);
}

class ConstantDecoderImpl : public ConstantDecoder {
  public:
    explicit ConstantDecoderImpl(const void *const data)
        : _constantSection(flatbuffers::GetRoot<const VGF::ConstantSection>(data)) {}

    [[nodiscard]] size_t size() const override {
        const auto *constants = _constantSection->data();
        return constants == nullptr ? 0 : constants->size();
    }

    [[nodiscard]] DataView<uint8_t> getConstant(uint32_t idx) const override {
        const auto *constants = _constantSection->data();
        return constants == nullptr || constants->Get(idx)->raw() == nullptr
                   ? DataView<uint8_t>()
                   : DataView<uint8_t>(constants->Get(idx)->raw()->data(), constants->Get(idx)->raw()->size());
    }

    [[nodiscard]] uint32_t getConstantMrtIndex(uint32_t idx) const override {
        const auto *constants = _constantSection->data();
        return constants == nullptr ? CONSTANT_INVALID_MRT_INDEX : constants->Get(idx)->mrt_index();
    }

    [[nodiscard]] bool isSparseConstant(uint32_t idx) const override {
        return getConstantSparsityDimension(idx) > CONSTANT_NOT_SPARSE_DIMENSION;
    }

    [[nodiscard]] int64_t getConstantSparsityDimension(uint32_t idx) const override {
        const auto *constants = _constantSection->data();
        if (constants == nullptr) {
            return CONSTANT_INVALID_SPARSITY_DIMENSION;
        }
        const auto dim = constants->Get(idx)->sparsity_dimension();
        if (dim < CONSTANT_NOT_SPARSE_DIMENSION) {
            return CONSTANT_INVALID_SPARSITY_DIMENSION;
        }
        return dim;
    }

  private:
    const VGF::ConstantSection *_constantSection;
};

class ConstantDecoder_V00_Impl : public ConstantDecoder {
  public:
    static std::unique_ptr<ConstantDecoder_V00_Impl> Create(const void *const data, const uint64_t sectionSize) {
        const auto verified = _verify(data, sectionSize);
        if (!verified.has_value()) {
            return nullptr;
        }
        const auto &[count, metaData, dataStart, dataSize] = verified.value();
        return std::unique_ptr<ConstantDecoder_V00_Impl>(
            new ConstantDecoder_V00_Impl(count, metaData, dataStart, dataSize));
    }

    static ConstantDecoder_V00_Impl *CreateInPlace(const void *const data, const uint64_t sectionSize,
                                                   void *const decoderMem) {
        const auto verified = _verify(data, sectionSize);
        if (!verified.has_value()) {
            return nullptr;
        }
        const auto &[count, metaData, dataStart, dataSize] = verified.value();
        return new (decoderMem) ConstantDecoder_V00_Impl(count, metaData, dataStart, dataSize);
    }

    [[nodiscard]] size_t size() const override { return static_cast<size_t>(_count); }

    [[nodiscard]] DataView<uint8_t> getConstant(uint32_t idx) const override {
        const auto *metaData = _getPtrToMetaData(idx);
        if (metaData == nullptr) {
            return {};
        }

        if (!_constantDataWithinBounds(metaData, _dataSize)) {
            return {};
        }

        return DataView<uint8_t>(_data + metaData->offset, static_cast<size_t>(metaData->size));
    }

    [[nodiscard]] uint32_t getConstantMrtIndex(uint32_t idx) const override {
        const auto *metaData = _getPtrToMetaData(idx);
        return metaData == nullptr ? CONSTANT_INVALID_MRT_INDEX : metaData->mrtIndex;
    }

    [[nodiscard]] int64_t getConstantSparsityDimension(uint32_t idx) const override {
        const auto *metaData = _getPtrToMetaData(idx);
        if (metaData == nullptr) {
            return CONSTANT_INVALID_SPARSITY_DIMENSION;
        }
        const auto dim = metaData->sparsityDimension;
        if (dim < CONSTANT_NOT_SPARSE_DIMENSION) {
            return CONSTANT_INVALID_SPARSITY_DIMENSION;
        }
        return dim;
    }

    [[nodiscard]] bool isSparseConstant(uint32_t idx) const override {
        return getConstantSparsityDimension(idx) > CONSTANT_NOT_SPARSE_DIMENSION;
    }

  private:
    explicit ConstantDecoder_V00_Impl(uint64_t count, const uint8_t *metaData, const uint8_t *data, uint64_t dataSize)
        : _count(count), _metaData(metaData), _data(data), _dataSize(dataSize) {}

    using VerifiedLayout = std::tuple<uint64_t, const uint8_t *, const uint8_t *, uint64_t>;

    [[nodiscard]] static std::optional<VerifiedLayout> _verify(const void *const data, const uint64_t sectionSize) {
        if (sectionSize < CONSTANT_SECTION_METADATA_OFFSET) {
            logging::error("Constant section too small to contain metadata");
            return std::nullopt;
        }

        const auto declaredCount = ReadBytesAs<uint64_t>(data, CONSTANT_SECTION_COUNT_OFFSET);
        const uint64_t maxEntries = (sectionSize - CONSTANT_SECTION_METADATA_OFFSET) / sizeof(ConstantMetaData_V00);
        if (declaredCount > maxEntries) {
            logging::error("Constant section declares more entries than fit in the buffer");
            return std::nullopt;
        }

        const uint64_t dataOffset = CONSTANT_SECTION_METADATA_OFFSET + declaredCount * sizeof(ConstantMetaData_V00);
        if (dataOffset > std::numeric_limits<size_t>::max()) {
            logging::error("Constant data offset exceeds addressable size");
            return std::nullopt;
        }
        if (dataOffset > sectionSize) {
            logging::error("Constant metadata exceeds section size");
            return std::nullopt;
        }

        const auto *metaData = static_cast<const uint8_t *>(data) + CONSTANT_SECTION_METADATA_OFFSET;
        const auto *dataStart = static_cast<const uint8_t *>(data) + static_cast<size_t>(dataOffset);
        const uint64_t dataSize = sectionSize - dataOffset;

        for (uint64_t idx = 0; idx < declaredCount; ++idx) {
            const auto *entry =
                reinterpret_cast<const ConstantMetaData_V00 *>(metaData + idx * sizeof(ConstantMetaData_V00));
            if (!_constantDataWithinBounds(entry, dataSize)) {
                logging::error("Constant metadata offset/size exceeds section bounds at index " + std::to_string(idx));
                return std::nullopt;
            }
        }

        return VerifiedLayout{declaredCount, metaData, dataStart, dataSize};
    }

    [[nodiscard]] const ConstantMetaData_V00 *_getPtrToMetaData(uint32_t idx) const {
        if (_metaData == nullptr || static_cast<uint64_t>(idx) >= _count) {
            return nullptr;
        }
        return reinterpret_cast<const ConstantMetaData_V00 *>(_metaData + idx * sizeof(ConstantMetaData_V00));
    }

    [[nodiscard]] static bool _constantDataWithinBounds(const ConstantMetaData_V00 *metaData, uint64_t dataSize) {
        if (metaData == nullptr) {
            return false;
        }
        const uint64_t offset = metaData->offset;
        const uint64_t entrySize = metaData->size;
        const uint64_t sizeMax = std::numeric_limits<size_t>::max();
        if (offset > sizeMax || entrySize > sizeMax || offset + entrySize > sizeMax) {
            return false;
        }

        if (offset > dataSize || entrySize > dataSize || entrySize > dataSize - offset) {
            return false;
        }
        return true;
    }

    uint64_t _count = 0;
    const uint8_t *_metaData = nullptr;
    const uint8_t *_data = nullptr;
    uint64_t _dataSize = 0;
};
size_t ConstantDecoderSize() { return std::max(sizeof(ConstantDecoderImpl), sizeof(ConstantDecoder_V00_Impl)); }

bool VerifyConstant(const void *data, const uint64_t size) {
    assert(data != nullptr && "data is null");
    std::unique_ptr<ConstantDecoder> decoder;

    if ((size >= CONSTANT_SECTION_VERSION_SIZE) &&
        strcmp(getConstantSectionVersion(data), CONSTANT_SECTION_VERSION) == 0) {
        auto decoderV00 = ConstantDecoder_V00_Impl::Create(data, size);
        if (decoderV00 == nullptr) {
            logging::error("Constant section could not be decoded safely");
            return false;
        }
        decoder = std::move(decoderV00);
    } else {
        if (!VerifyImpl<VGF::ConstantSection>(data, size)) {
            logging::error("Constant section could not be decoded safely");
            return false;
        }
        decoder = std::make_unique<ConstantDecoderImpl>(data);
    }

    for (size_t i = 0; i < decoder->size(); ++i) {
        if (decoder->getConstantSparsityDimension(static_cast<uint32_t>(i)) == CONSTANT_INVALID_SPARSITY_DIMENSION) {
            logging::error("Constant sparsity dimension is invalid at index " + std::to_string(i));
            return false;
        }
    }

    return true;
}

std::unique_ptr<ConstantDecoder> CreateConstantDecoder(const void *const data, const uint64_t size) {
    assert(data != nullptr && "data is null");
    if (size < CONSTANT_SECTION_VERSION_SIZE) {
        logging::error("Constant section too small to contain version");
        return {};
    }
    if (strcmp(getConstantSectionVersion(data), CONSTANT_SECTION_VERSION) == 0) {
        auto decoder = ConstantDecoder_V00_Impl::Create(data, size);
        if (decoder == nullptr) {
            logging::error("Constant section verification failed");
            return {};
        }
        return decoder;
    }
    return std::make_unique<ConstantDecoderImpl>(data);
}

ConstantDecoder *CreateConstantDecoderInPlace(const void *const data, const uint64_t size, void *decoderMem) {
    assert(data != nullptr && "data is null");
    assert(decoderMem != nullptr && "decoderMem is null");
    if (size < CONSTANT_SECTION_VERSION_SIZE) {
        logging::error("Constant section too small to contain version");
        return nullptr;
    }
    if (strcmp(getConstantSectionVersion(data), CONSTANT_SECTION_VERSION) == 0) {
        auto *decoder = ConstantDecoder_V00_Impl::CreateInPlace(data, size, decoderMem);
        if (decoder == nullptr) {
            logging::error("Constant section verification failed");
            return nullptr;
        }
        return decoder;
    }
    return new (decoderMem) ConstantDecoderImpl(data);
}

} // namespace mlsdk::vgflib
