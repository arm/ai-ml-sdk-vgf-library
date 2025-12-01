/*
 * SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf/encoder.hpp"

#include "constant.hpp"
#include "header.hpp"
#include "internal_logging.hpp"
#include "internal_types.hpp"
#include "section_index_table.hpp"
#include "vgf_generated.h"

#include <fstream>

namespace mlsdk::vgflib {

namespace {

VGF::ModuleType toVGF(ModuleType type) {
    switch (type) {
    case ModuleType::COMPUTE:
        return VGF::ModuleType::ModuleType_COMPUTE;
    case ModuleType::GRAPH:
        return VGF::ModuleType::ModuleType_GRAPH;
    default:
        assert(false && "unknown Moduletype");
        return VGF::ModuleType::ModuleType_COMPUTE;
    }
}

VGF::ResourceCategory toVGF(ResourceCategory category) {
    switch (category) {
    case ResourceCategory::INPUT:
        return VGF::ResourceCategory::ResourceCategory_INPUT;
    case ResourceCategory::OUTPUT:
        return VGF::ResourceCategory::ResourceCategory_OUTPUT;
    case ResourceCategory::INTERMEDIATE:
        return VGF::ResourceCategory::ResourceCategory_INTERMEDIATE;
    case ResourceCategory::CONSTANT:
        return VGF::ResourceCategory::ResourceCategory_CONSTANT;
    default:
        assert(false && "unknown ResourceCategory");
        return VGF::ResourceCategory::ResourceCategory_MAX;
    }
}

template <typename T> size_t getAlignedSize(size_t size) { return (size + sizeof(T) - 1) / sizeof(T) * sizeof(T); }

} // namespace

class EncoderImpl : public Encoder {
  public:
    explicit EncoderImpl(uint16_t vkHeaderVersion) : _vkHeaderVersion(vkHeaderVersion) {}

    ModuleRef AddModule(ModuleType type, const std::string &name, const std::string &entryPoint,
                        const std::vector<uint32_t> &code) override {
        assert(!_finished && "cannot add modules when marked finished");
        if (code.empty()) {
            _modules.emplace_back(
                VGF::CreateModuleDirect(_moduleBuilder, toVGF(type), name.c_str(), entryPoint.c_str()));
        } else {
            auto spirv = VGF::CreateSPIRVDirect(_moduleBuilder, &code);
            _modules.emplace_back(VGF::CreateModuleDirect(_moduleBuilder, toVGF(type), name.c_str(), entryPoint.c_str(),
                                                          VGF::ModuleCode::ModuleCode_SPIRV, spirv.Union()));
        }
        auto moduleRef = static_cast<ModuleRef::RefType>(_modules.size() - 1);
        logging::debug("Added module. Name: " + name + " EntryPoint: " + entryPoint +
                       " Type: " + std::to_string(static_cast<int>(type)) + " ModuleRef: " + std::to_string(moduleRef));
        _moduleRefToType.push_back(type);
        assert(moduleRef == _moduleRefToType.size() - 1);
        assert(_modules.size() == _moduleRefToType.size());
        return {moduleRef};
    }

    ModuleRef AddPlaceholderModule(ModuleType type, const std::string &name, const std::string &entryPoint) override {
        return AddModule(type, name, entryPoint, {});
    }

    BindingSlotRef AddBindingSlot(uint32_t binding, ResourceRef resource) override {
        assert(!_finished && "cannot add binding slots when marked finished");

        _bindingSlots.emplace_back(VGF::CreateBindingSlot(_modelSequenceBuilder, binding, resource.reference));

        return {static_cast<uint32_t>(_bindingSlots.size() - 1)};
    }

    DescriptorSetInfoRef AddDescriptorSetInfo(const std::vector<BindingSlotRef> &bindings) override {
        assert(!_finished && "cannot add descriptor set infos when marked finished");

        auto bindingOffsets = _modelSequenceBuilder.CreateVector<flatbuffers::Offset<VGF::BindingSlot>>(
            bindings.size(), [this, &bindings](size_t i) { return _bindingSlots[bindings[i].reference]; });

        _descriptorSetInfos.emplace_back(VGF::CreateDescriptorSetInfo(_modelSequenceBuilder, bindingOffsets));

        return {static_cast<uint32_t>(_descriptorSetInfos.size() - 1)};
    }

    PushConstRangeRef AddPushConstRange(uint32_t stageFlags, uint32_t offset, uint32_t size) override {
        assert(!_finished && "cannot add push constant range when marked finished");

        _pushConstRanges.emplace_back(VGF::CreatePushConstantRange(_modelSequenceBuilder, stageFlags, offset, size));

        return {static_cast<uint32_t>(_pushConstRanges.size() - 1)};
    }

    SegmentInfoRef AddSegmentInfo(ModuleRef module, const std::string &name,
                                  const std::vector<DescriptorSetInfoRef> &descriptors,
                                  const std::vector<BindingSlotRef> &inputs, const std::vector<BindingSlotRef> &outputs,
                                  const std::vector<ConstantRef> &constants,
                                  const std::array<uint32_t, 3> &dispatchShape,
                                  const std::vector<PushConstRangeRef> &pushConstRanges) override {

        assert(!_finished && "cannot add segment infos when marked finished");

        ModuleType type = _moduleRefToType[module.reference];

        auto constantOffsets = _modelSequenceBuilder.CreateVector<uint32_t>(
            constants.size(), [&constants](size_t i) { return constants[i].reference; });
        auto dispatchShapeOffsets = _modelSequenceBuilder.CreateVector(dispatchShape.data(), dispatchShape.size());

        auto descriptorSetOffsets = _modelSequenceBuilder.CreateVector<flatbuffers::Offset<VGF::DescriptorSetInfo>>(
            descriptors.size(),
            [this, &descriptors](size_t i) { return _descriptorSetInfos[descriptors[i].reference]; });

        auto inputOffsets = _modelSequenceBuilder.CreateVector<flatbuffers::Offset<VGF::BindingSlot>>(
            inputs.size(), [this, &inputs](size_t i) { return _bindingSlots[inputs[i].reference]; });

        auto outputOffsets = _modelSequenceBuilder.CreateVector<flatbuffers::Offset<VGF::BindingSlot>>(
            outputs.size(), [this, &outputs](size_t i) { return _bindingSlots[outputs[i].reference]; });

        auto pushConstRangeOffsets = _modelSequenceBuilder.CreateVector<flatbuffers::Offset<VGF::PushConstantRange>>(
            pushConstRanges.size(),
            [this, &pushConstRanges](size_t i) { return _pushConstRanges[pushConstRanges[i].reference]; });

        _segmentInfos.emplace_back(
            VGF::CreateSegmentInfo(_modelSequenceBuilder, toVGF(type), _modelSequenceBuilder.CreateString(name.c_str()),
                                   module.reference, descriptorSetOffsets, inputOffsets, outputOffsets, constantOffsets,
                                   dispatchShapeOffsets, pushConstRangeOffsets));

        const auto segmentRef = static_cast<SegmentInfoRef::RefType>(_segmentInfos.size() - 1);
        logging::debug("Added segment info. Name: " + name + " ModuleRef: " + std::to_string(module.reference) +
                       " SegmentRef: " + std::to_string(segmentRef));
        return {segmentRef};
    }

    void AddModelSequenceInputsOutputs(const std::vector<BindingSlotRef> &inputs,
                                       const std::vector<std::string> &inputNames,
                                       const std::vector<BindingSlotRef> &outputs,
                                       const std::vector<std::string> &outputNames) override {

        assert(!_finished && "cannot add input/output binding slots when marked finished");

        std::copy(inputs.begin(), inputs.end(), std::back_inserter(_modelSequenceInputs));
        std::copy(outputs.begin(), outputs.end(), std::back_inserter(_modelSequenceOutputs));

        auto f = [&](auto &s) { return _modelSequenceBuilder.CreateString(s); };
        std::transform(inputNames.begin(), inputNames.end(), std::back_inserter(_inputNames), f);
        std::transform(outputNames.begin(), outputNames.end(), std::back_inserter(_outputNames), f);
    }

    ResourceRef AddModelResourceTableEntry(ResourceCategory category, std::optional<DescriptorType> vkDescriptorType,
                                           FormatType vkFormat, const std::vector<int64_t> &shape,
                                           const std::vector<int64_t> &strides) {
        assert(!_finished && "cannot add resource when marked as finished");

        EncodedDescriptorType encodedDescType =
            vkDescriptorType ? static_cast<EncodedDescriptorType>(*vkDescriptorType) : NullOptDescriptorType();

        auto description = VGF::CreateDescriptionDirect(_modelResourceBuilder, &shape, &strides);
        _resources.emplace_back(VGF::CreateModelResourceTableEntry(
            _modelResourceBuilder, encodedDescType, static_cast<uint32_t>(vkFormat), toVGF(category), description));
        return {static_cast<uint32_t>(_resources.size() - 1)};
    }

    ResourceRef AddInputResource(DescriptorType vkDescriptorType, FormatType vkFormat,
                                 const std::vector<int64_t> &shape, const std::vector<int64_t> &strides) override {
        return AddModelResourceTableEntry(ResourceCategory::INPUT, vkDescriptorType, vkFormat, shape, strides);
    }

    ResourceRef AddOutputResource(DescriptorType vkDescriptorType, FormatType vkFormat,
                                  const std::vector<int64_t> &shape, const std::vector<int64_t> &strides) override {
        return AddModelResourceTableEntry(ResourceCategory::OUTPUT, vkDescriptorType, vkFormat, shape, strides);
    }

    ResourceRef AddIntermediateResource(DescriptorType vkDescriptorType, FormatType vkFormat,
                                        const std::vector<int64_t> &shape,
                                        const std::vector<int64_t> &strides) override {
        return AddModelResourceTableEntry(ResourceCategory::INTERMEDIATE, vkDescriptorType, vkFormat, shape, strides);
    }

    ResourceRef AddConstantResource(FormatType vkFormat, const std::vector<int64_t> &shape,
                                    const std::vector<int64_t> &strides) override {
        return AddModelResourceTableEntry(ResourceCategory::CONSTANT, std::nullopt, vkFormat, shape, strides);
    }

    ConstantRef AddConstant(ResourceRef resourceRef, const void *data, size_t sizeInBytes,
                            int64_t sparsityDimension) override {
        assert(!_finished && "cannot add constants when marked finished");
        assert(data && "data pointer cannot be nullptr");
        assert(sizeInBytes > 0 && "sizeInBytes cannot be zero");

        _constsMetaData.emplace_back(ConstantMetaData_V00{
            resourceRef.reference,
            static_cast<int32_t>(sparsityDimension),
            sizeInBytes,
            _constDataOffset,
        });

        uint64_t sizeInBytesAligned = getAlignedSize<uint64_t>(sizeInBytes);
        auto &constantData = _constsData.emplace_back(sizeInBytesAligned, 0);
        std::memcpy(constantData.data(), data, sizeInBytes);
        _constDataOffset += sizeInBytesAligned;

        return {static_cast<uint32_t>(_constsMetaData.size() - 1)};
    }

    void Finish() override {
        assert(!_finished && "already marked finished");

        auto moduleSection = VGF::CreateModuleTable(_moduleBuilder, _moduleBuilder.CreateVector(_modules));
        _moduleBuilder.Finish(moduleSection);

        auto modelResourceTable =
            VGF::CreateModelResourceTable(_modelResourceBuilder, _modelResourceBuilder.CreateVector(_resources));
        _modelResourceBuilder.Finish(modelResourceTable);

        auto modelSequenceInputOffsets = _modelSequenceBuilder.CreateVector<flatbuffers::Offset<VGF::BindingSlot>>(
            _modelSequenceInputs.size(), [this](size_t i) { return _bindingSlots[_modelSequenceInputs[i].reference]; });

        auto modelSequenceOutputOffsets = _modelSequenceBuilder.CreateVector<flatbuffers::Offset<VGF::BindingSlot>>(
            _modelSequenceOutputs.size(),
            [this](size_t i) { return _bindingSlots[_modelSequenceOutputs[i].reference]; });

        auto inputNamesOffsets =
            _modelSequenceBuilder.CreateVector<flatbuffers::Offset<flatbuffers::String>>(_inputNames);

        auto outputNamesOffsets =
            _modelSequenceBuilder.CreateVector<flatbuffers::Offset<flatbuffers::String>>(_outputNames);

        auto modelSequenceSection = VGF::CreateModelSequenceTable(
            _modelSequenceBuilder, _modelSequenceBuilder.CreateVector(_segmentInfos), modelSequenceInputOffsets,
            modelSequenceOutputOffsets, inputNamesOffsets, outputNamesOffsets);

        _modelSequenceBuilder.Finish(modelSequenceSection);
        _finished = true;
    }

    bool WriteTo(std::ostream &output) override {
        assert(_finished && "cannot write if encoding is not marked finished");
        logging::debug("Writing VGF model to output stream");

        SectionIndexTable table;
        const auto &headerSection = table.AddSection(sizeof(Header));
        const auto &moduleSection = table.AddSection(_moduleBuilder.GetSize());
        const auto &modelSequenceSection = table.AddSection(_modelSequenceBuilder.GetSize());
        const auto &modelResourceSection = table.AddSection(_modelResourceBuilder.GetSize());

        auto numConsts = static_cast<uint64_t>(_constsMetaData.size());
        const auto constantSectionSize =
            CONSTANT_SECTION_METADATA_OFFSET + numConsts * sizeof(ConstantMetaData_V00) + _constDataOffset;
        const auto &constantSection = table.AddSection(constantSectionSize);

        // calculate alignments and offsets
        table.Update();

        Header header(moduleSection, modelSequenceSection, modelResourceSection, constantSection, _vkHeaderVersion);

        if (!headerSection.Write(output, &header)) {
            logging::error("Failed to write header section");
            return false;
        }
        if (!moduleSection.Write(output, _moduleBuilder.GetBufferPointer())) {
            logging::error("Failed to write module section");
            return false;
        }
        if (!modelSequenceSection.Write(output, _modelSequenceBuilder.GetBufferPointer())) {
            logging::error("Failed to write model sequence section");
            return false;
        }
        if (!modelResourceSection.Write(output, _modelResourceBuilder.GetBufferPointer())) {
            logging::error("Failed to write model resource section");
            return false;
        }

        output.write(CONSTANT_SECTION_VERSION, CONSTANT_SECTION_VERSION_SIZE);
        output.write(reinterpret_cast<const char *>(&numConsts), CONSTANT_SECTION_COUNT_SIZE);
        output.write(reinterpret_cast<const char *>(_constsMetaData.data()),
                     static_cast<std::streamsize>(numConsts * sizeof(ConstantMetaData_V00)));
        for (auto &constsData : _constsData) {
            output.write(reinterpret_cast<const char *>(constsData.data()),
                         static_cast<std::streamsize>(constsData.size()));
            // Erasing each constant immediately after writing it to disk to release memory
            constsData.clear();
            if (output.fail()) {
                logging::error("Failed to write constant section, rdstate: " +
                               std::string(rdStateToStr(output.rdstate())));
                return false;
            }
        }
        _constsData.clear();

        return true;
    }

  private:
    bool _finished = false;
    flatbuffers::FlatBufferBuilder _moduleBuilder;
    flatbuffers::FlatBufferBuilder _modelSequenceBuilder;
    flatbuffers::FlatBufferBuilder _modelResourceBuilder;

    std::vector<flatbuffers::Offset<VGF::Module>> _modules;
    std::vector<flatbuffers::Offset<VGF::ModelResourceTableEntry>> _resources;
    std::vector<flatbuffers::Offset<VGF::BindingSlot>> _bindingSlots;
    std::vector<flatbuffers::Offset<VGF::DescriptorSetInfo>> _descriptorSetInfos;
    std::vector<flatbuffers::Offset<VGF::SegmentInfo>> _segmentInfos;
    std::vector<flatbuffers::Offset<VGF::PushConstantRange>> _pushConstRanges;
    std::vector<flatbuffers::Offset<flatbuffers::String>> _inputNames;
    std::vector<flatbuffers::Offset<flatbuffers::String>> _outputNames;
    std::vector<BindingSlotRef> _modelSequenceInputs;
    std::vector<BindingSlotRef> _modelSequenceOutputs;
    std::vector<ModuleType> _moduleRefToType;

    std::vector<ConstantMetaData_V00> _constsMetaData;
    std::list<std::vector<uint8_t>> _constsData;
    uint64_t _constDataOffset = 0;

    uint16_t _vkHeaderVersion;
};

std::unique_ptr<Encoder> CreateEncoder(uint16_t vkHeaderVersion) {
    return std::make_unique<EncoderImpl>(vkHeaderVersion);
}

} // namespace mlsdk::vgflib
