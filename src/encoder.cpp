/*
 * SPDX-FileCopyrightText: Copyright 2023-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf/encoder.hpp"

#include "constant.hpp"
#include "header.hpp"
#include "internal_logging.hpp"
#include "internal_types.hpp"
#include "section_index_table.hpp"
#include "utils.hpp"
#include "vgf_generated.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <limits>

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

VGF::ModuleCode toVGF(ShaderType type) {
    switch (type) {
    case ShaderType::GLSL:
        return VGF::ModuleCode::ModuleCode_GLSL;
    case ShaderType::HLSL:
        return VGF::ModuleCode::ModuleCode_HLSL;
    default:
        assert(false && "unknown Moduletype");
        return VGF::ModuleCode::ModuleCode_NONE;
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

} // namespace

class EncoderImpl : public Encoder {
  public:
    explicit EncoderImpl(uint16_t vkHeaderVersion) : vkHeaderVersion_(vkHeaderVersion) {}

    ModuleRef AddModule(ModuleType type, const std::string &name, const std::string &entryPoint,
                        const std::vector<uint32_t> &code) override {
        assert(!finished_ && "cannot add modules when marked finished");
        if (code.empty()) {
            modules_.emplace_back(VGF::CreateModuleDirect(moduleBuilder_, toVGF(type), name.c_str(), entryPoint.c_str(),
                                                          VGF::ModuleCode::ModuleCode_SPIRV));
        } else {
            auto spirv = VGF::CreateSPIRVDirect(moduleBuilder_, &code);
            modules_.emplace_back(VGF::CreateModuleDirect(moduleBuilder_, toVGF(type), name.c_str(), entryPoint.c_str(),
                                                          VGF::ModuleCode::ModuleCode_SPIRV, spirv.Union()));
        }
        auto moduleRef = static_cast<ModuleRef::RefType>(modules_.size() - 1);
        logging::debug("Added module. Name: " + name + " EntryPoint: " + entryPoint +
                       " Type: " + std::to_string(static_cast<int>(type)) + " ModuleRef: " + std::to_string(moduleRef));
        moduleRefToType_.push_back(type);
        assert(moduleRef == moduleRefToType_.size() - 1);
        assert(modules_.size() == moduleRefToType_.size());
        return {moduleRef};
    }

    ModuleRef AddModule(ModuleType moduleType, const std::string &name, const std::string &entryPoint,
                        ShaderType shaderType, const std::string &code) override {
        assert(!finished_ && "cannot add modules when marked finished");

        if (code.empty()) {
            modules_.emplace_back(VGF::CreateModuleDirect(moduleBuilder_, toVGF(moduleType), name.c_str(),
                                                          entryPoint.c_str(), toVGF(shaderType)));
        } else {
            switch (shaderType) {
            case ShaderType::GLSL: {
                auto glsl = VGF::CreateGLSLDirect(moduleBuilder_, code.c_str());
                modules_.emplace_back(VGF::CreateModuleDirect(moduleBuilder_, toVGF(moduleType), name.c_str(),
                                                              entryPoint.c_str(), toVGF(shaderType), glsl.Union()));
                break;
            }
            case ShaderType::HLSL: {
                auto hlsl = VGF::CreateHLSLDirect(moduleBuilder_, code.c_str());
                modules_.emplace_back(VGF::CreateModuleDirect(moduleBuilder_, toVGF(moduleType), name.c_str(),
                                                              entryPoint.c_str(), toVGF(shaderType), hlsl.Union()));
                break;
            }
            default:
                assert(false && "unknown shader type");
                break;
            }
        }
        auto moduleRef = static_cast<ModuleRef::RefType>(modules_.size() - 1);
        logging::debug("Added module. Name: " + name + " EntryPoint: " + entryPoint + " Type: " +
                       std::to_string(static_cast<int>(moduleType)) + " ModuleRef: " + std::to_string(moduleRef));
        moduleRefToType_.push_back(moduleType);
        assert(moduleRef == moduleRefToType_.size() - 1);
        assert(modules_.size() == moduleRefToType_.size());
        return {moduleRef};
    }

    ModuleRef AddPlaceholderModule(ModuleType type, const std::string &name, const std::string &entryPoint) override {
        logging::warning("AddPlaceholderModule is deprecated use AddModule instead");
        return AddModule(type, name, entryPoint, {});
    }

    BindingSlotRef AddBindingSlot(uint32_t binding, ResourceRef resource) override {
        assert(!finished_ && "cannot add binding slots when marked finished");

        bindingSlots_.emplace_back(VGF::CreateBindingSlot(modelSequenceBuilder_, binding, resource.reference));

        return {static_cast<uint32_t>(bindingSlots_.size() - 1)};
    }

    DescriptorSetInfoRef AddDescriptorSetInfo(const std::vector<BindingSlotRef> &bindings, uint32_t setIndex) override {
        assert(!finished_ && "cannot add descriptor set infos when marked finished");

        auto bindingOffsets = modelSequenceBuilder_.CreateVector<flatbuffers::Offset<VGF::BindingSlot>>(
            bindings.size(), [this, &bindings](size_t i) { return bindingSlots_[bindings[i].reference]; });

        descriptorSetInfos_.emplace_back(VGF::CreateDescriptorSetInfo(modelSequenceBuilder_, bindingOffsets, setIndex));

        return {static_cast<uint32_t>(descriptorSetInfos_.size() - 1)};
    }

    PushConstRangeRef AddPushConstRange(uint32_t stageFlags, uint32_t offset, uint32_t size) override {
        assert(!finished_ && "cannot add push constant range when marked finished");

        pushConstRanges_.emplace_back(VGF::CreatePushConstantRange(modelSequenceBuilder_, stageFlags, offset, size));

        return {static_cast<uint32_t>(pushConstRanges_.size() - 1)};
    }

    SegmentInfoRef AddSegmentInfo(ModuleRef module, const std::string &name,
                                  const std::vector<DescriptorSetInfoRef> &descriptors,
                                  const std::vector<BindingSlotRef> &inputs, const std::vector<BindingSlotRef> &outputs,
                                  const std::vector<ConstantRef> &constants,
                                  const std::array<uint32_t, 3> &dispatchShape,
                                  const std::vector<PushConstRangeRef> &pushConstRanges) override {

        assert(!finished_ && "cannot add segment infos when marked finished");

        ModuleType type = moduleRefToType_[module.reference];

        auto constantOffsets = modelSequenceBuilder_.CreateVector<uint32_t>(
            constants.size(), [&constants](size_t i) { return constants[i].reference; });
        auto dispatchShapeOffsets = modelSequenceBuilder_.CreateVector(dispatchShape.data(), dispatchShape.size());

        auto descriptorSetOffsets = modelSequenceBuilder_.CreateVector<flatbuffers::Offset<VGF::DescriptorSetInfo>>(
            descriptors.size(),
            [this, &descriptors](size_t i) { return descriptorSetInfos_[descriptors[i].reference]; });

        auto inputOffsets = modelSequenceBuilder_.CreateVector<flatbuffers::Offset<VGF::BindingSlot>>(
            inputs.size(), [this, &inputs](size_t i) { return bindingSlots_[inputs[i].reference]; });

        auto outputOffsets = modelSequenceBuilder_.CreateVector<flatbuffers::Offset<VGF::BindingSlot>>(
            outputs.size(), [this, &outputs](size_t i) { return bindingSlots_[outputs[i].reference]; });

        auto pushConstRangeOffsets = modelSequenceBuilder_.CreateVector<flatbuffers::Offset<VGF::PushConstantRange>>(
            pushConstRanges.size(),
            [this, &pushConstRanges](size_t i) { return pushConstRanges_[pushConstRanges[i].reference]; });

        segmentInfos_.emplace_back(
            VGF::CreateSegmentInfo(modelSequenceBuilder_, toVGF(type), modelSequenceBuilder_.CreateString(name.c_str()),
                                   module.reference, descriptorSetOffsets, inputOffsets, outputOffsets, constantOffsets,
                                   dispatchShapeOffsets, pushConstRangeOffsets));

        const auto segmentRef = static_cast<SegmentInfoRef::RefType>(segmentInfos_.size() - 1);
        logging::debug("Added segment info. Name: " + name + " ModuleRef: " + std::to_string(module.reference) +
                       " SegmentRef: " + std::to_string(segmentRef));
        return {segmentRef};
    }

    void AddModelSequenceInputsOutputs(const std::vector<BindingSlotRef> &inputs,
                                       const std::vector<std::string> &inputNames,
                                       const std::vector<BindingSlotRef> &outputs,
                                       const std::vector<std::string> &outputNames) override {

        assert(!finished_ && "cannot add input/output binding slots when marked finished");

        std::copy(inputs.begin(), inputs.end(), std::back_inserter(modelSequenceInputs_));
        std::copy(outputs.begin(), outputs.end(), std::back_inserter(modelSequenceOutputs_));

        auto f = [&](auto &s) { return modelSequenceBuilder_.CreateString(s); };
        std::transform(inputNames.begin(), inputNames.end(), std::back_inserter(inputNames_), f);
        std::transform(outputNames.begin(), outputNames.end(), std::back_inserter(outputNames_), f);
    }

    ResourceRef AddModelResourceTableEntry(ResourceCategory category, std::optional<DescriptorType> vkDescriptorType,
                                           FormatType vkFormat, const std::vector<int64_t> &shape,
                                           const std::vector<int64_t> &strides,
                                           std::optional<AliasGroupId> aliasGroupId = std::nullopt) {
        assert(!finished_ && "cannot add resource when marked as finished");
        assert((!aliasGroupId.has_value() || *aliasGroupId != INVALID_ALIAS_GROUP_ID) &&
               "aliasGroupId must not use the reserved invalid value");
        resourceRecords_.emplace_back(ResourceRecord{
            category,
            vkDescriptorType,
            vkFormat,
            shape,
            strides,
            aliasGroupId,
            std::nullopt,
        });
        return {static_cast<uint32_t>(resourceRecords_.size() - 1)};
    }

    ResourceRef AddInputResource(DescriptorType vkDescriptorType, FormatType vkFormat,
                                 const std::vector<int64_t> &shape, const std::vector<int64_t> &strides,
                                 std::optional<AliasGroupId> aliasGroupId) override {
        return AddModelResourceTableEntry(ResourceCategory::INPUT, vkDescriptorType, vkFormat, shape, strides,
                                          aliasGroupId);
    }

    ResourceRef AddOutputResource(DescriptorType vkDescriptorType, FormatType vkFormat,
                                  const std::vector<int64_t> &shape, const std::vector<int64_t> &strides,
                                  std::optional<AliasGroupId> aliasGroupId) override {
        return AddModelResourceTableEntry(ResourceCategory::OUTPUT, vkDescriptorType, vkFormat, shape, strides,
                                          aliasGroupId);
    }

    ResourceRef AddIntermediateResource(DescriptorType vkDescriptorType, FormatType vkFormat,
                                        const std::vector<int64_t> &shape, const std::vector<int64_t> &strides,
                                        std::optional<AliasGroupId> aliasGroupId) override {
        return AddModelResourceTableEntry(ResourceCategory::INTERMEDIATE, vkDescriptorType, vkFormat, shape, strides,
                                          aliasGroupId);
    }

    ResourceRef AddConstantResource(FormatType vkFormat, const std::vector<int64_t> &shape,
                                    const std::vector<int64_t> &strides) override {
        return AddModelResourceTableEntry(ResourceCategory::CONSTANT, std::nullopt, vkFormat, shape, strides);
    }

    void AddSamplerConfig(ResourceRef resource, uint32_t samplerMinFilter, uint32_t samplerMagFilter,
                          uint32_t samplerAddressModeU, uint32_t samplerAddressModeV,
                          uint32_t samplerBorderColor) override {
        assert(!finished_ && "cannot add sampler config when marked as finished");
        assert(resource.reference < resourceRecords_.size() && "resource reference out of range");
        resourceRecords_[resource.reference].samplerConfig = SamplerConfigRecord{
            samplerMinFilter, samplerMagFilter, samplerAddressModeU, samplerAddressModeV, samplerBorderColor,
        };
    }

    void SetAliasGroup(ResourceRef resource, AliasGroupId aliasGroupId) override {
        assert(!finished_ && "cannot set alias group when marked as finished");
        assert(resource.reference < resourceRecords_.size() && "resource reference out of range");
        assert(aliasGroupId != INVALID_ALIAS_GROUP_ID && "aliasGroupId must not use the reserved invalid value");

        auto &resourceRecord = resourceRecords_[resource.reference];
        assert(resourceRecord.category != ResourceCategory::CONSTANT &&
               "constant resources do not support alias groups");
        if (resourceRecord.aliasGroupId.has_value()) {
            assert(*resourceRecord.aliasGroupId == aliasGroupId &&
                   "resource already belongs to a different alias group");
            return;
        }

        resourceRecord.aliasGroupId = aliasGroupId;
    }

    ConstantRef AddConstant(ResourceRef resourceRef, const void *data, size_t sizeInBytes,
                            int64_t sparsityDimension) override {
        assert(!finished_ && "cannot add constants when marked finished");
        assert(data && "data pointer cannot be nullptr");
        assert(sizeInBytes > 0 && "sizeInBytes cannot be zero");

        constexpr auto MIN_SPARSITY_DIM = INT32_MIN_VALUE;
        constexpr auto MAX_SPARSITY_DIM = INT32_MAX_VALUE;
        int32_t sparsityDim32{};
        if (sparsityDimension < MIN_SPARSITY_DIM || sparsityDimension > MAX_SPARSITY_DIM) {
            logging::error("sparsityDimension must fit in int32_t for on-disk metadata; clamping to range");
            sparsityDim32 = static_cast<int32_t>(
                std::clamp<int64_t>(sparsityDimension, int64_t{MIN_SPARSITY_DIM}, int64_t{MAX_SPARSITY_DIM}));
        } else {
            sparsityDim32 = static_cast<int32_t>(sparsityDimension);
        }

        const auto sizeInBytesAligned = checkedAlignUp(static_cast<uint64_t>(sizeInBytes), sizeof(uint64_t));
        if (!sizeInBytesAligned.has_value()) {
            logging::error("Constant size exceeds addressable on-disk metadata size");
            encodingFailed_ = true;
            return {UINT32_MAX_VALUE};
        }
        if (*sizeInBytesAligned > SIZE_MAX_VALUE) {
            logging::error("Constant size exceeds addressable host allocation size");
            encodingFailed_ = true;
            return {UINT32_MAX_VALUE};
        }
        const auto nextDataOffset = checkedAdd(constDataOffset_, *sizeInBytesAligned);
        if (!nextDataOffset.has_value()) {
            logging::error("Constant data section size exceeds addressable on-disk metadata size");
            encodingFailed_ = true;
            return {UINT32_MAX_VALUE};
        }

        constsMetaData_.emplace_back(ConstantMetaDataV00{
            resourceRef.reference,
            sparsityDim32,
            sizeInBytes,
            constDataOffset_,
        });

        auto &constantData = constsData_.emplace_back(static_cast<size_t>(*sizeInBytesAligned), 0);
        std::memcpy(constantData.data(), data, sizeInBytes);
        constDataOffset_ = *nextDataOffset;

        return {static_cast<uint32_t>(constsMetaData_.size() - 1)};
    }

    void Finish() override {
        assert(!finished_ && "already marked finished");

        auto moduleSection = VGF::CreateModuleTable(moduleBuilder_, moduleBuilder_.CreateVector(modules_));
        moduleBuilder_.Finish(moduleSection);

        auto modelResourceTableEntries =
            modelResourceBuilder_.CreateVector<flatbuffers::Offset<VGF::ModelResourceTableEntry>>(
                resourceRecords_.size(), [this](size_t i) {
                    const auto &resource = resourceRecords_[i];
                    EncodedDescriptorType encodedDescType =
                        resource.vkDescriptorType ? static_cast<EncodedDescriptorType>(*resource.vkDescriptorType)
                                                  : NullOptDescriptorType();
                    const uint32_t encodedAliasGroupId = resource.aliasGroupId.value_or(INVALID_ALIAS_GROUP_ID);
                    auto description =
                        VGF::CreateDescriptionDirect(modelResourceBuilder_, &resource.shape, &resource.strides);
                    VGF::ExtraConfig extraConfigType = VGF::ExtraConfig_NONE;
                    flatbuffers::Offset<void> extraConfig{};
                    if (resource.samplerConfig.has_value()) {
                        const SamplerConfigRecord &config = *resource.samplerConfig;
                        auto samplerConfig =
                            VGF::CreateSamplerConfig(modelResourceBuilder_, config.minFilter, config.magFilter,
                                                     config.addressModeU, config.addressModeV, config.borderColor);
                        extraConfigType = VGF::ExtraConfig_SamplerConfig;
                        extraConfig = samplerConfig.Union();
                    }
                    return VGF::CreateModelResourceTableEntry(
                        modelResourceBuilder_, encodedDescType, static_cast<uint32_t>(resource.vkFormat),
                        toVGF(resource.category), description, extraConfigType, extraConfig, encodedAliasGroupId);
                });
        auto modelResourceTable = VGF::CreateModelResourceTable(modelResourceBuilder_, modelResourceTableEntries);
        modelResourceBuilder_.Finish(modelResourceTable);

        auto modelSequenceInputOffsets = modelSequenceBuilder_.CreateVector<flatbuffers::Offset<VGF::BindingSlot>>(
            modelSequenceInputs_.size(), [this](size_t i) { return bindingSlots_[modelSequenceInputs_[i].reference]; });

        auto modelSequenceOutputOffsets = modelSequenceBuilder_.CreateVector<flatbuffers::Offset<VGF::BindingSlot>>(
            modelSequenceOutputs_.size(),
            [this](size_t i) { return bindingSlots_[modelSequenceOutputs_[i].reference]; });

        auto inputNamesOffsets =
            modelSequenceBuilder_.CreateVector<flatbuffers::Offset<flatbuffers::String>>(inputNames_);

        auto outputNamesOffsets =
            modelSequenceBuilder_.CreateVector<flatbuffers::Offset<flatbuffers::String>>(outputNames_);

        auto modelSequenceSection = VGF::CreateModelSequenceTable(
            modelSequenceBuilder_, modelSequenceBuilder_.CreateVector(segmentInfos_), modelSequenceInputOffsets,
            modelSequenceOutputOffsets, inputNamesOffsets, outputNamesOffsets);

        modelSequenceBuilder_.Finish(modelSequenceSection);
        finished_ = true;
    }

    bool WriteTo(std::ostream &output) override {
        assert(finished_ && "cannot write if encoding is not marked finished");
        logging::debug("Writing VGF model to output stream");
        if (encodingFailed_) {
            logging::error("Cannot write VGF model after encoder failed");
            return false;
        }

        SectionIndexTable table;
        const auto &headerSection = table.AddSection(sizeof(Header));
        const auto &moduleSection = table.AddSection(moduleBuilder_.GetSize());
        const auto &modelSequenceSection = table.AddSection(modelSequenceBuilder_.GetSize());
        const auto &modelResourceSection = table.AddSection(modelResourceBuilder_.GetSize());

        auto numConsts = static_cast<uint64_t>(constsMetaData_.size());
        const auto constantMetadataSize = checkedMul(numConsts, sizeof(ConstantMetaDataV00));
        const auto constantHeaderAndMetadataSize =
            constantMetadataSize.has_value() ? checkedAdd(CONSTANT_SECTION_METADATA_OFFSET, *constantMetadataSize)
                                             : std::optional<uint64_t>{};
        const auto constantSectionSize = constantHeaderAndMetadataSize.has_value()
                                             ? checkedAdd(*constantHeaderAndMetadataSize, constDataOffset_)
                                             : std::optional<uint64_t>{};
        if (!constantSectionSize.has_value()) {
            logging::error("Constant section size exceeds addressable on-disk metadata size");
            return false;
        }
        const auto &constantSection = table.AddSection(*constantSectionSize);

        // calculate alignments and offsets
        table.Update();

        Header header(moduleSection, modelSequenceSection, modelResourceSection, constantSection, vkHeaderVersion_);

        if (!headerSection.Write(output, &header)) {
            logging::error("Failed to write header section");
            return false;
        }
        if (!moduleSection.Write(output, moduleBuilder_.GetBufferPointer())) {
            logging::error("Failed to write module section");
            return false;
        }
        if (!modelSequenceSection.Write(output, modelSequenceBuilder_.GetBufferPointer())) {
            logging::error("Failed to write model sequence section");
            return false;
        }
        if (!modelResourceSection.Write(output, modelResourceBuilder_.GetBufferPointer())) {
            logging::error("Failed to write model resource section");
            return false;
        }

        output.write(CONSTANT_SECTION_VERSION, CONSTANT_SECTION_VERSION_SIZE);
        output.write(reinterpret_cast<const char *>(&numConsts), CONSTANT_SECTION_COUNT_SIZE);
        output.write(reinterpret_cast<const char *>(constsMetaData_.data()),
                     static_cast<std::streamsize>(numConsts * sizeof(ConstantMetaDataV00)));
        for (auto &constsData : constsData_) {
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
        constsData_.clear();

        return true;
    }

  private:
    struct SamplerConfigRecord {
        uint32_t minFilter;
        uint32_t magFilter;
        uint32_t addressModeU;
        uint32_t addressModeV;
        uint32_t borderColor;
    };

    struct ResourceRecord {
        ResourceCategory category;
        std::optional<DescriptorType> vkDescriptorType;
        FormatType vkFormat;
        std::vector<int64_t> shape;
        std::vector<int64_t> strides;
        std::optional<AliasGroupId> aliasGroupId;
        std::optional<SamplerConfigRecord> samplerConfig;
    };

    bool finished_ = false;
    bool encodingFailed_ = false;
    flatbuffers::FlatBufferBuilder moduleBuilder_;
    flatbuffers::FlatBufferBuilder modelSequenceBuilder_;
    flatbuffers::FlatBufferBuilder modelResourceBuilder_;

    std::vector<flatbuffers::Offset<VGF::Module>> modules_;
    std::vector<ResourceRecord> resourceRecords_;
    std::vector<flatbuffers::Offset<VGF::BindingSlot>> bindingSlots_;
    std::vector<flatbuffers::Offset<VGF::DescriptorSetInfo>> descriptorSetInfos_;
    std::vector<flatbuffers::Offset<VGF::SegmentInfo>> segmentInfos_;
    std::vector<flatbuffers::Offset<VGF::PushConstantRange>> pushConstRanges_;
    std::vector<flatbuffers::Offset<flatbuffers::String>> inputNames_;
    std::vector<flatbuffers::Offset<flatbuffers::String>> outputNames_;
    std::vector<BindingSlotRef> modelSequenceInputs_;
    std::vector<BindingSlotRef> modelSequenceOutputs_;
    std::vector<ModuleType> moduleRefToType_;

    std::vector<ConstantMetaDataV00> constsMetaData_;
    std::list<std::vector<uint8_t>> constsData_;
    uint64_t constDataOffset_ = 0;

    uint16_t vkHeaderVersion_;
};

std::unique_ptr<Encoder> CreateEncoder(uint16_t vkHeaderVersion) {
    return std::make_unique<EncoderImpl>(vkHeaderVersion);
}

} // namespace mlsdk::vgflib
