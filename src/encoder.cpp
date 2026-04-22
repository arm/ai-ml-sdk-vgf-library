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
#include "vgf_generated.h"

#include <algorithm>
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

template <typename T> size_t getAlignedSize(size_t size) { return (size + sizeof(T) - 1) / sizeof(T) * sizeof(T); }

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
                                           const std::vector<int64_t> &strides) {
        assert(!finished_ && "cannot add resource when marked as finished");

        EncodedDescriptorType encodedDescType =
            vkDescriptorType ? static_cast<EncodedDescriptorType>(*vkDescriptorType) : NullOptDescriptorType();

        auto description = VGF::CreateDescriptionDirect(modelResourceBuilder_, &shape, &strides);
        resources_.emplace_back(VGF::CreateModelResourceTableEntry(
            modelResourceBuilder_, encodedDescType, static_cast<uint32_t>(vkFormat), toVGF(category), description));
        return {static_cast<uint32_t>(resources_.size() - 1)};
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

        constsMetaData_.emplace_back(ConstantMetaDataV00{
            resourceRef.reference,
            sparsityDim32,
            sizeInBytes,
            constDataOffset_,
        });

        uint64_t sizeInBytesAligned = getAlignedSize<uint64_t>(sizeInBytes);
        auto &constantData = constsData_.emplace_back(sizeInBytesAligned, 0);
        std::memcpy(constantData.data(), data, sizeInBytes);
        constDataOffset_ += sizeInBytesAligned;

        return {static_cast<uint32_t>(constsMetaData_.size() - 1)};
    }

    void Finish() override {
        assert(!finished_ && "already marked finished");

        auto moduleSection = VGF::CreateModuleTable(moduleBuilder_, moduleBuilder_.CreateVector(modules_));
        moduleBuilder_.Finish(moduleSection);

        auto modelResourceTable =
            VGF::CreateModelResourceTable(modelResourceBuilder_, modelResourceBuilder_.CreateVector(resources_));
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

        SectionIndexTable table;
        const auto &headerSection = table.AddSection(sizeof(Header));
        const auto &moduleSection = table.AddSection(moduleBuilder_.GetSize());
        const auto &modelSequenceSection = table.AddSection(modelSequenceBuilder_.GetSize());
        const auto &modelResourceSection = table.AddSection(modelResourceBuilder_.GetSize());

        auto numConsts = static_cast<uint64_t>(constsMetaData_.size());
        const auto constantSectionSize =
            CONSTANT_SECTION_METADATA_OFFSET + numConsts * sizeof(ConstantMetaDataV00) + constDataOffset_;
        const auto &constantSection = table.AddSection(constantSectionSize);

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
    bool finished_ = false;
    flatbuffers::FlatBufferBuilder moduleBuilder_;
    flatbuffers::FlatBufferBuilder modelSequenceBuilder_;
    flatbuffers::FlatBufferBuilder modelResourceBuilder_;

    std::vector<flatbuffers::Offset<VGF::Module>> modules_;
    std::vector<flatbuffers::Offset<VGF::ModelResourceTableEntry>> resources_;
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
