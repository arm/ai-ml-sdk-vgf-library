/*
 * SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf/decoder.hpp"

#include "header.hpp"
#include "internal_types.hpp"
#include "vgf_generated.h"

#include <cassert>

namespace mlsdk::vgflib {
namespace {

template <typename T> inline T ReadBytesAs(const void *const baseAddr, size_t offset) {
    const uint8_t *bytes = reinterpret_cast<const uint8_t *const>(baseAddr) + offset;
    return *reinterpret_cast<const T *const>(bytes);
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

} // namespace

// Header Decoder
class HeaderDecoderImpl : public HeaderDecoder {
  public:
    explicit HeaderDecoderImpl(const void *const data) : _header(reinterpret_cast<const Header *>(data)) {}

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

std::unique_ptr<HeaderDecoder> CreateHeaderDecoder(const void *const data) {
    assert(data != nullptr && "data is null");
    return std::make_unique<HeaderDecoderImpl>(data);
}

HeaderDecoder *CreateHeaderDecoderInPlace(const void *const data, void *decoderMem) {
    assert(data != nullptr && "data is null");
    assert(decoderMem != nullptr && "decoderMem is null");
    return new (decoderMem) HeaderDecoderImpl(data);
}

// Module Table decoder
class ModuleTableDecoderImpl : public ModuleTableDecoder {
  public:
    explicit ModuleTableDecoderImpl(const void *const data)
        : _moduleTable(flatbuffers::GetRoot<const VGF::ModuleTable>(data)) {}

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

std::unique_ptr<ModuleTableDecoder> CreateModuleTableDecoder(const void *const data) {
    assert(data != nullptr && "data is null");
    return std::make_unique<ModuleTableDecoderImpl>(data);
}

ModuleTableDecoder *CreateModuleTableDecoderInPlace(const void *const data, void *decoderMem) {
    assert(data != nullptr && "data is null");
    assert(decoderMem != nullptr && "decoderMem is null");
    return new (decoderMem) ModuleTableDecoderImpl(data);
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
    explicit ModelSequenceTableDecoderImpl(const void *const data)
        : _modelSequenceTable(flatbuffers::GetRoot<const VGF::ModelSequenceTable>(data)) {}

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

std::unique_ptr<ModelSequenceTableDecoder> CreateModelSequenceTableDecoder(const void *const data) {
    assert(data != nullptr && "data is null");
    return std::make_unique<ModelSequenceTableDecoderImpl>(data);
}

ModelSequenceTableDecoder *CreateModelSequenceTableDecoderInPlace(const void *const data, void *decoderMem) {
    assert(data != nullptr && "data is null");
    assert(decoderMem != nullptr && "decoderMem is null");
    return new (decoderMem) ModelSequenceTableDecoderImpl(data);
}

// Model Resource Table Decoder
class ModelResourceTableDecoderImpl : public ModelResourceTableDecoder {
  public:
    explicit ModelResourceTableDecoderImpl(const void *const data)
        : _modelRecTable(flatbuffers::GetRoot<const VGF::ModelResourceTable>(data)) {}

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

std::unique_ptr<ModelResourceTableDecoder> CreateModelResourceTableDecoder(const void *const data) {
    assert(data != nullptr && "data is null");
    return std::make_unique<ModelResourceTableDecoderImpl>(data);
}

ModelResourceTableDecoder *CreateModelResourceTableDecoderInPlace(const void *const data, void *decoderMem) {
    assert(data != nullptr && "data is null");
    assert(decoderMem != nullptr && "decoderMem is null");
    return new (decoderMem) ModelResourceTableDecoderImpl(data);
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
        return constants == nullptr ? 0 : constants->Get(idx)->mrt_index();
    }

    [[nodiscard]] bool isSparseConstant(uint32_t idx) const override {
        const auto *constants = _constantSection->data();
        return constants == nullptr ? false : constants->Get(idx)->sparsity_dimension() != -1;
    }

    [[nodiscard]] int64_t getConstantSparsityDimension(uint32_t idx) const override {
        const auto *constants = _constantSection->data();
        return constants == nullptr ? -1 : constants->Get(idx)->sparsity_dimension();
    }

  private:
    const VGF::ConstantSection *_constantSection;
};

size_t ConstantDecoderSize() { return sizeof(ConstantDecoderImpl); }

bool VerifyConstant(const void *data, const uint64_t size) {
    assert(data != nullptr && "data is null");
    return VerifyImpl<VGF::ConstantSection>(data, size);
}

std::unique_ptr<ConstantDecoder> CreateConstantDecoder(const void *const data) {
    assert(data != nullptr && "data is null");
    return std::make_unique<ConstantDecoderImpl>(data);
}

ConstantDecoder *CreateConstantDecoderInPlace(const void *const data, void *decoderMem) {
    assert(data != nullptr && "data is null");
    assert(decoderMem != nullptr && "decoderMem is null");
    return new (decoderMem) ConstantDecoderImpl(data);
}

} // namespace mlsdk::vgflib
