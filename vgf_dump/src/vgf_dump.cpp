/*
 * SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <vgf/vgf_dump.hpp>

#include <vgf/decoder.hpp>
#include <vgf/types.hpp>

#include "parse_vgf.hpp"
#include <vgf-utils/memory_map.hpp>
#include <vgf-utils/numpy.hpp>

#define VGFLIB_VK_HELPERS // Avoid need to include Vulkan headers
#include <vgf/vulkan_helpers.generated.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#    include <windows.h>

#    include <fcntl.h>
#    include <fileapi.h>
#    include <io.h>
#    include <stdio.h>
#else
#    include <sys/mman.h>
#    include <unistd.h>
#endif

using nlohmann::json;
using namespace mlsdk::vgflib;

namespace {

using mlsdk::vgfutils::BindingSlot;
using mlsdk::vgfutils::Constant;
using mlsdk::vgfutils::ModelSequence;
using mlsdk::vgfutils::NamedBindingSlot;
using mlsdk::vgfutils::parseModelResourceTable;
using mlsdk::vgfutils::parseModelSequenceTable;
using mlsdk::vgfutils::PushConstantRange;
using mlsdk::vgfutils::Resource;
using mlsdk::vgfutils::Segment;

std::string moduleTypeToString(ModuleType type) {
    switch (type) {
    case ModuleType::COMPUTE:
        return "COMPUTE";
    case ModuleType::GRAPH:
        return "GRAPH";
    default:
        throw std::runtime_error("Unknown module type");
    }
}

std::string resourceCategoryToString(ResourceCategory category) {
    switch (category) {
    case ResourceCategory::INPUT:
        return "INPUT";
    case ResourceCategory::OUTPUT:
        return "OUTPUT";
    case ResourceCategory::INTERMEDIATE:
        return "INTERMEDIATE";
    case ResourceCategory::CONSTANT:
        return "CONSTANT";
    default:
        throw std::runtime_error("Unknown ResourceCategory");
    }
}

std::string DescriptorTypeToString(std::optional<DescriptorType> type) {
    if (!type.has_value()) {
        return "none";
    }
    return DescriptorTypeToName(type.value());
}

std::string FormatTypeToString(FormatType format) { return FormatTypeToName(format); }

struct Header {
    Header() = default;
    Header(uint8_t major, uint8_t minor, uint8_t patch) : mMajor(major), mMinor(minor), mPatch(patch) {}

    uint8_t mMajor{0};
    uint8_t mMinor{0};
    uint8_t mPatch{0};
};

void to_json(json &j, const Header &header) {
    j = json{
        {"major", header.mMajor},
        {"minor", header.mMinor},
        {"patch", header.mPatch},
    };
}

struct Module {
    Module() = default;
    Module(uint32_t index, ModuleType type, std::string_view &&name, std::string_view &&entryPoint, bool hasSPIRV,
           size_t codeSize)
        : mIndex(index), mType(type), mName(name), mEntryPoint(entryPoint), mHasSPIRV(hasSPIRV), mCodeSize(codeSize) {}

    uint32_t mIndex{0};
    ModuleType mType{ModuleType::COMPUTE};
    std::string mName;
    std::string mEntryPoint;
    bool mHasSPIRV{false};
    size_t mCodeSize{0};
};

void to_json(json &j, const Module &m) {
    j = json{
        {"index", m.mIndex},        {"type", moduleTypeToString(m.mType)},
        {"name", m.mName},          {"entry_point", m.mEntryPoint},
        {"has_spirv", m.mHasSPIRV}, {"code_size", m.mCodeSize},
    };
}

std::unique_ptr<HeaderDecoder> parseHeader(const void *const headerData) {
    std::unique_ptr<HeaderDecoder> headerDecoder = CreateHeaderDecoder(headerData);
    if (!headerDecoder->IsValid()) {
        throw std::runtime_error("Invalid VGF header, bad magic value");
    }
    if (!headerDecoder->CheckVersion()) {
        throw std::runtime_error("Unsupported VGF file version: " + std::to_string(headerDecoder->GetMajor()) + "." +
                                 std::to_string(headerDecoder->GetMinor()) + "." +
                                 std::to_string(headerDecoder->GetPatch()));
    }
    return headerDecoder;
}

std::vector<Module> parseModuleTable(const void *const data) {
    std::unique_ptr<ModuleTableDecoder> decoder = CreateModuleTableDecoder(data);

    std::vector<Module> modules;
    modules.reserve(decoder->size());
    for (uint32_t i = 0; i < decoder->size(); i++) {
        modules.emplace_back(i, decoder->getModuleType(i), decoder->getModuleName(i), decoder->getModuleEntryPoint(i),
                             decoder->hasSPIRV(i), decoder->getModuleCode(i).size());
    }
    return modules;
}

void writeOutputText(const std::string &path, const char *ptr, size_t size) {
    if (path == "-") {
        std::cout << std::string_view(ptr, size);
        return;
    }

    std::ios_base::openmode flags = std::ios::out | std::ios::trunc;
    std::ofstream file(path, flags);
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.write(ptr, static_cast<std::streamsize>(size));
}

void writeOutputBinary(const std::string &path, const char *ptr, size_t size) {
    if (path == "-") {
#ifdef _WIN32
        _setmode(_fileno(stdout), _O_BINARY);
#endif
        std::cout << std::string_view(ptr, size);
#ifdef _WIN32
        _setmode(_fileno(stdout), _O_TEXT);
#endif
        return;
    }

    std::ios_base::openmode flags = std::ios::out | std::ios::trunc | std::ios::binary;
    std::ofstream file(path, flags);
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.write(ptr, static_cast<std::streamsize>(size));
}

void writeOutputJSON(const std::string &path, json &json) {
    std::string jsonContents = json.dump(/*indent=*/4);
    writeOutputText(path, jsonContents.c_str(), jsonContents.size());
}

struct ScenarioTensorResource {
    ScenarioTensorResource(const std::string &name, const std::string &uid, const std::string &path, bool isSrc,
                           VkFormat format, const DataView<int64_t> &dims)
        : mName(name), mUid(uid), mPath(path), mIsSrc(isSrc), mFormat(format), mDims(dims.begin(), dims.end()) {}

    ScenarioTensorResource(const std::string &uid, const std::string &path, bool isInput, VkFormat format,
                           const DataView<int64_t> &dims)
        : ScenarioTensorResource("tensor", uid, path, isInput, format, dims){};

    std::string mName;
    std::string mUid;
    std::string mPath;
    bool mIsSrc;
    VkFormat mFormat;
    std::vector<uint64_t> mDims;
};

void to_json(json &j, const ScenarioTensorResource &tensor) {
    if (tensor.mName == "graph") {
        j = json{{tensor.mName, {{"uid", tensor.mUid}, {tensor.mIsSrc ? "src" : "dst", tensor.mPath}}}};
    } else {
        j = json{{tensor.mName,
                  {
                      {"uid", tensor.mUid},
                      {tensor.mIsSrc ? "src" : "dst", tensor.mPath},
                      {"shader_access", tensor.mIsSrc ? "readonly" : "writeonly"},
                      {"format", FormatTypeToString(tensor.mFormat)},
                      {"dims", tensor.mDims},
                  }}};
    }
}

struct ScenarioGraphResource {
    ScenarioGraphResource(const std::string &name, const std::string &uid, const std::string &path, bool isSrc,
                          VkFormat format, const DataView<int64_t> &dims)
        : mName(name), mUid(uid), mPath(path), mIsSrc(isSrc), mFormat(format), mDims(dims.begin(), dims.end()) {}

    ScenarioGraphResource(const std::string &uid, const std::string &src)
        : ScenarioGraphResource("graph", uid, src, true, UndefinedFormat(), {}){};

    std::string mName;
    std::string mUid;
    std::string mPath;
    bool mIsSrc;
    VkFormat mFormat;
    std::vector<uint64_t> mDims;
};

void to_json(json &j, const ScenarioGraphResource &graph) {
    if (graph.mName == "graph") {
        j = json{{graph.mName, {{"uid", graph.mUid}, {graph.mIsSrc ? "src" : "dst", graph.mPath}}}};
    } else {
        j = json{{graph.mName,
                  {
                      {"uid", graph.mUid},
                      {graph.mIsSrc ? "src" : "dst", graph.mPath},
                      {"shader_access", graph.mIsSrc ? "readonly" : "writeonly"},
                      {"format", FormatTypeToString(graph.mFormat)},
                      {"dims", graph.mDims},
                  }}};
    }
}

struct ScenarioShaderResource {
    ScenarioShaderResource(const std::string &name, const std::string &uid, const std::string &path,
                           const std::string &type, const std::string &entry)
        : mName(name), mUid(uid), mPath(path), mType(type), mEntry(entry) {}

    ScenarioShaderResource(const std::string &uid, const std::string &src, const std::string &type,
                           const std::string &entry)
        : ScenarioShaderResource("shader", uid, src, type, entry){};

    std::string mName;
    std::string mUid;
    std::string mPath;
    std::string mType;
    std::string mEntry;
};

void to_json(json &j, const ScenarioShaderResource &shader) {
    if (shader.mName == "shader") {
        j = json{{shader.mName,
                  {{"uid", shader.mUid}, {"src", shader.mPath}, {"type", shader.mType}, {"entry", shader.mEntry}}}};
    } else {
        j = json{{shader.mName,
                  {
                      {"uid", shader.mUid},
                      {"src", shader.mPath},
                      {"type", shader.mType},
                      {"entry", shader.mEntry},
                  }}};
    }
}

struct ScenarioBinding {
    ScenarioBinding(std::string uid, uint32_t id, uint32_t descriptorSet = 0)
        : mUid(std::move(uid)), mId(id), mDescriptorSet(descriptorSet) {}
    std::string mUid;
    uint32_t mId;
    uint32_t mDescriptorSet;
};

void to_json(json &j, const ScenarioBinding &binding) {
    j = json{
        {"resource_ref", binding.mUid},
        {"id", binding.mId},
        {"set", binding.mDescriptorSet},
    };
}
struct ScenarioShaderSubstitutions {
    ScenarioShaderSubstitutions(std::string shaderRef, std::string target)
        : mShaderRef(std::move(shaderRef)), mTarget(std::move(target)) {}
    std::string mShaderRef;
    std::string mTarget;
};

void to_json(json &j, const ScenarioShaderSubstitutions &shader_substitution) {
    j = json{
        {"shader_ref", shader_substitution.mShaderRef},
        {"target", shader_substitution.mTarget},
    };
}

struct Boundary {
    Boundary(std::vector<std::string> resources, uint32_t index)
        : mResources(std::move(resources)), mIndex(std::move(index)) {}
    std::vector<std::string> mResources;
    uint32_t mIndex;
};

void to_json(json &j, const Boundary &boundary) {
    j = json{{"mark_boundary",
              {
                  {"resources", boundary.mResources},
                  {"frame_id", boundary.mIndex},
              }}};
}

} // namespace

namespace mlsdk::vgfutils {

void to_json(nlohmann::json &j, const BindingSlot &bindingSlot) {
    j = nlohmann::json{
        {"index", bindingSlot.mIndex}, {"binding", bindingSlot.mBinding}, {"mrt_index", bindingSlot.mMrtIndex}};
}

void to_json(nlohmann::json &j, const PushConstantRange &pushConstantRange) {
    j = nlohmann::json{{"index", pushConstantRange.mIndex},
                       {"stage_flags", pushConstantRange.mStageFlags},
                       {"offset", pushConstantRange.mOffset},
                       {"size", pushConstantRange.mSize}};
}

void to_json(nlohmann::json &j, const Segment &segment) {
    j = nlohmann::json{{"index", segment.mIndex},
                       {"type", moduleTypeToString(segment.mType)},
                       {"module_index", segment.mModuleIndex},
                       {"name", segment.mName},
                       {"inputs", segment.mInputs},
                       {"outputs", segment.mOutputs},
                       {"descriptor_set_infos", segment.mDescriptorSetInfos},
                       {"push_constant_ranges", segment.mPushConstantRanges},
                       {"constants", segment.mConstants},
                       {"dispatch_shape", segment.mDispatchShape}};
}

void to_json(nlohmann::json &j, const NamedBindingSlot &namedSlot) {
    j = nlohmann::json{{"index", namedSlot.mBindingSlot.mIndex},
                       {"name", namedSlot.mName},
                       {"binding", namedSlot.mBindingSlot.mBinding},
                       {"mrt_index", namedSlot.mBindingSlot.mMrtIndex}};
}

void to_json(nlohmann::json &j, const ModelSequence &modelSequence) {
    j = nlohmann::json{
        {"segments", modelSequence.mSegments}, {"inputs", modelSequence.mInputs}, {"outputs", modelSequence.mOutputs}};
}

void to_json(nlohmann::json &j, const Constant &constant) {
    j = nlohmann::json{{"index", constant.mIndex},
                       {"mrt_index", constant.mMrtIndex},
                       {"sparsity_dimension", constant.mSparsityDimension}};
}

void to_json(nlohmann::json &j, const Resource &resource) {
    j = json{
        {"index", resource.mIndex},
        {"category", resourceCategoryToString(resource.mCategory)},
        {"vk_descriptor_type", DescriptorTypeToString(resource.mDescriptorType)},
        {"vk_format", FormatTypeToString(resource.mVkFormat)},
        {"shape", resource.mShape},
        {"stride", resource.mStride},
    };
}

} // namespace mlsdk::vgfutils

void mlsdk::vgf_dump::dumpSpirv(const std::string &inputFile, const std::string &outputFile, uint32_t index) {
    getSpirv(inputFile, index, [&](const uint32_t *data, size_t size) {
        writeOutputBinary(outputFile, reinterpret_cast<const char *>(data), size * sizeof(uint32_t));
    });
}

void mlsdk::vgf_dump::dumpConstant(const std::string &inputFile, const std::string &outputFile, uint32_t index) {
    getConstant(inputFile, index, [&](const uint8_t *data, size_t size) {
        writeOutputBinary(outputFile, reinterpret_cast<const char *>(data), size);
    });
}

void mlsdk::vgf_dump::dumpNumpy(const std::string &inputFile, const std::string &outputFile, uint32_t index) {
    MemoryMap mapped(inputFile);
    const auto headerDecoder = parseHeader(mapped.ptr());

    if (!VerifyConstant(mapped.ptr(headerDecoder->GetConstantsOffset()), headerDecoder->GetConstantsSize())) {
        throw std::runtime_error("Invalid header data");
    }
    if (!VerifyModelResourceTable(mapped.ptr(headerDecoder->GetModelResourceTableOffset()),
                                  headerDecoder->GetModelResourceTableSize())) {
        throw std::runtime_error("Invalid header data");
    }

    const auto constantDecoder = CreateConstantDecoder(mapped.ptr(headerDecoder->GetConstantsOffset()));
    const auto mrtDecoder = CreateModelResourceTableDecoder(mapped.ptr(headerDecoder->GetModelResourceTableOffset()));

    if (index >= constantDecoder->size()) {
        throw std::runtime_error("Constant index " + std::to_string(index) +
                                 " out of bounds. Number of constants: " + std::to_string(constantDecoder->size()));
    }

    const auto mrtIndex = constantDecoder->getConstantMrtIndex(index);
    const auto format = mrtDecoder->getVkFormat(mrtIndex);
    const auto shapeView = mrtDecoder->getTensorShape(mrtIndex);
    const std::vector<int64_t> shape(shapeView.begin(), shapeView.end());
    const auto data = constantDecoder->getConstant(index);

    const auto numeric = componentNumericFormat(format);
    const auto encoding = mlsdk::vgfutils::numpy::numpyTypeEncoding(numeric);
    const auto itemsize = mlsdk::vgfutils::numpy::elementSizeFromBlockSize(blockSize(format));

    mlsdk::vgfutils::numpy::write(outputFile, reinterpret_cast<const char *>(data.begin()), shape, encoding, itemsize);
}

void mlsdk::vgf_dump::dumpScenario(const std::string &inputFile, const std::string &outputFile, bool add_boundaries) {
    json json = getScenario(inputFile, add_boundaries);
    writeOutputJSON(outputFile, json);
}

void mlsdk::vgf_dump::dumpFile(const std ::string &inputFile, const std::string &outputFile) {
    json json = getFile(inputFile);
    writeOutputJSON(outputFile, json);
}

void mlsdk::vgf_dump::getSpirv(const std::string &inputFile, uint32_t index,
                               std::function<void(const uint32_t *, size_t)> callback) {
    MemoryMap mapped(inputFile);
    std::unique_ptr<HeaderDecoder> headerDecoder = parseHeader(mapped.ptr());

    if (!VerifyModuleTable(mapped.ptr(headerDecoder->GetModuleTableOffset()), headerDecoder->GetModuleTableSize())) {
        throw std::runtime_error("Invalid module table");
    }

    std::unique_ptr<ModuleTableDecoder> decoder =
        CreateModuleTableDecoder(mapped.ptr(headerDecoder->GetModuleTableOffset()));

    if (index >= decoder->size()) {
        throw std::runtime_error("Module index " + std::to_string(index) +
                                 " out of bounds. Number of modules: " + std::to_string(decoder->size()));
    }
    if (!decoder->hasSPIRV(index)) {
        throw std::runtime_error("Module index " + std::to_string(index) + " has no stored code");
    }
    DataView<uint32_t> data = decoder->getModuleCode(index);
    callback(&data[0], data.size());
}

void mlsdk::vgf_dump::getConstant(const std::string &inputFile, uint32_t index,
                                  std::function<void(const uint8_t *, size_t)> callback) {
    MemoryMap mapped(inputFile);
    std::unique_ptr<HeaderDecoder> headerDecoder = parseHeader(mapped.ptr());

    if (!VerifyConstant(mapped.ptr(headerDecoder->GetConstantsOffset()), headerDecoder->GetConstantsSize())) {
        throw std::runtime_error("Invalid header data");
    }

    std::unique_ptr<ConstantDecoder> decoder = CreateConstantDecoder(mapped.ptr(headerDecoder->GetConstantsOffset()));

    if (index >= decoder->size()) {
        throw std::runtime_error("Constant index " + std::to_string(index) +
                                 " out of bounds. Number of constants: " + std::to_string(decoder->size()));
    }

    DataView<uint8_t> data = decoder->getConstant(index);
    callback(&data[0], data.size());
}

json mlsdk::vgf_dump::getScenario(const std::string &inputFile, bool add_boundaries) {
    MemoryMap mapped(inputFile);

    std::unique_ptr<HeaderDecoder> headerDecoder = parseHeader(mapped.ptr());

    std::vector<ScenarioBinding> bindings;
    std::vector<ScenarioGraphResource> graphResources;
    graphResources.push_back(
        ScenarioGraphResource("vgf_graph_ref", std::filesystem::path(inputFile).filename().string()));

    if (!VerifyModelResourceTable(mapped.ptr(headerDecoder->GetModelResourceTableOffset()),
                                  headerDecoder->GetModelResourceTableSize())) {
        throw std::runtime_error("Invalid model resource table");
    }
    if (!VerifyModelSequenceTable(mapped.ptr(headerDecoder->GetModelSequenceTableOffset()),
                                  headerDecoder->GetModelSequenceTableSize())) {
        throw std::runtime_error("Invalid model sequeqnce table");
    }

    std::unique_ptr<ModelResourceTableDecoder> modelResourceDecoder =
        CreateModelResourceTableDecoder(mapped.ptr(headerDecoder->GetModelResourceTableOffset()));
    std::unique_ptr<ModelSequenceTableDecoder> modelSequenceDecoder =
        CreateModelSequenceTableDecoder(mapped.ptr(headerDecoder->GetModelSequenceTableOffset()));

    std::vector<ScenarioTensorResource> tensorResources;
    BindingSlotArrayHandle seqInputsHandle = modelSequenceDecoder->getModelSequenceInputBindingSlotsHandle();
    for (uint32_t i = 0; i < modelSequenceDecoder->getBindingsSize(seqInputsHandle); ++i) {
        std::string uid = "input_" + std::to_string(i) + "_ref";
        bindings.emplace_back(uid, modelSequenceDecoder->getBindingSlotBinding(seqInputsHandle, i));
        uint32_t mrtIndex = modelSequenceDecoder->getBindingSlotMrtIndex(seqInputsHandle, i);

        if (modelResourceDecoder->getCategory(mrtIndex) != ResourceCategory::INPUT) {
            throw std::runtime_error("VGF Input has a mismatched ResourceCategory");
        }

        std::string descTypeName = DescriptorTypeToString(modelResourceDecoder->getDescriptorType(mrtIndex));
        if (descTypeName == "VK_DESCRIPTOR_TYPE_TENSOR_ARM") {
            tensorResources.push_back(ScenarioTensorResource(uid, "TEMPLATE_PATH_TENSOR_INPUT_" + std::to_string(i),
                                                             true, modelResourceDecoder->getVkFormat(mrtIndex),
                                                             modelResourceDecoder->getTensorShape(mrtIndex)));
        } else {
            std::stringstream ss;
            ss << "Not implemented descriptor type support " << descTypeName
               << " for input resource when creating the scenario";
            throw std::runtime_error(ss.str());
        }
    }

    std::vector<std::string> outputs;
    BindingSlotArrayHandle seqOutputsHandle = modelSequenceDecoder->getModelSequenceOutputBindingSlotsHandle();
    for (uint32_t i = 0; i < modelSequenceDecoder->getBindingsSize(seqOutputsHandle); ++i) {
        std::string uid = "output_" + std::to_string(i) + "_ref";
        outputs.emplace_back(uid);
        bindings.emplace_back(uid, modelSequenceDecoder->getBindingSlotBinding(seqOutputsHandle, i));
        uint32_t mrtIndex = modelSequenceDecoder->getBindingSlotMrtIndex(seqOutputsHandle, i);

        if (modelResourceDecoder->getCategory(mrtIndex) != ResourceCategory::OUTPUT) {
            throw std::runtime_error("VGF Output has a mismatched ResourceCategory");
        }

        std::string descTypeName = DescriptorTypeToString(modelResourceDecoder->getDescriptorType(mrtIndex));
        if (descTypeName == "VK_DESCRIPTOR_TYPE_TENSOR_ARM") {
            tensorResources.push_back(ScenarioTensorResource(uid, "TEMPLATE_PATH_TENSOR_OUTPUT_" + std::to_string(i),
                                                             false, modelResourceDecoder->getVkFormat(mrtIndex),
                                                             modelResourceDecoder->getTensorShape(mrtIndex)));
        } else {
            std::stringstream ss;
            ss << "Not implemented descriptor type support " << descTypeName
               << " for output resource when creating the scenario";
            throw std::runtime_error(ss.str());
        }
    }

    if (!VerifyModuleTable(mapped.ptr(headerDecoder->GetModuleTableOffset()), headerDecoder->GetModuleTableSize())) {
        throw std::runtime_error("Invalid module table");
    }
    uint32_t shaderIdx = 0;
    std::vector<ScenarioShaderSubstitutions> shader_substitutions;
    std::vector<ScenarioShaderResource> shaderResources;
    for (auto &module : parseModuleTable(mapped.ptr(headerDecoder->GetModuleTableOffset()))) {

        if (module.mType == ModuleType::COMPUTE) {
            std::string shaderRef = "shader_" + std::to_string(shaderIdx) + "_ref";
            shader_substitutions.emplace_back(shaderRef, std::string(module.mName));

            shaderResources.push_back(ScenarioShaderResource(
                shaderRef, "TEMPLATE_PATH_SHADER_GLSL_" + std::to_string(shaderIdx), "GLSL", module.mEntryPoint));
            shaderIdx++;
        }
    }

    std::vector<json> commands;
    if (add_boundaries) {
        commands.push_back(Boundary(std::vector<std::string>{}, 0));
    }
    commands.push_back(json{{"dispatch_graph",
                             {
                                 {"bindings", bindings},
                                 {"shader_substitutions", shader_substitutions},
                                 {"graph_ref", "vgf_graph_ref"},
                             }}});
    if (add_boundaries) {
        commands.push_back(Boundary(outputs, 1));
    }

    json json;

    for (auto &graphResource : graphResources)
        json["resources"].push_back(graphResource);
    for (auto &tensorResource : tensorResources)
        json["resources"].push_back(tensorResource);
    for (auto &shaderResource : shaderResources)
        json["resources"].push_back(shaderResource);

    json["commands"] = commands;

    return json;
}

json mlsdk::vgf_dump::getFile(const std::string &inputFile) {
    MemoryMap mapped(inputFile);

    std::unique_ptr<HeaderDecoder> headerDecoder = parseHeader(mapped.ptr());
    Header header(headerDecoder->GetMajor(), headerDecoder->GetMinor(), headerDecoder->GetPatch());

    if (!VerifyModuleTable(mapped.ptr(headerDecoder->GetModuleTableOffset()), headerDecoder->GetModuleTableSize())) {
        throw std::runtime_error("Invalid module table");
    }
    if (!VerifyModelResourceTable(mapped.ptr(headerDecoder->GetModelResourceTableOffset()),
                                  headerDecoder->GetModelResourceTableSize())) {
        throw std::runtime_error("Invalid model resource table");
    }
    if (!VerifyModelSequenceTable(mapped.ptr(headerDecoder->GetModelSequenceTableOffset()),
                                  headerDecoder->GetModelSequenceTableSize())) {
        throw std::runtime_error("Invalid model sequence table");
    }
    if (!VerifyConstant(mapped.ptr(headerDecoder->GetConstantsOffset()), headerDecoder->GetConstantsSize())) {
        throw std::runtime_error("Invalid constant section");
    }

    std::vector<Module> modules = parseModuleTable(mapped.ptr(headerDecoder->GetModuleTableOffset()));
    std::vector<Resource> resources = parseModelResourceTable(mapped.ptr(headerDecoder->GetModelResourceTableOffset()));
    ModelSequence modelSequence = parseModelSequenceTable(mapped.ptr(headerDecoder->GetModelSequenceTableOffset()));

    std::unique_ptr<ConstantDecoder> constantDecoder =
        CreateConstantDecoder(mapped.ptr(headerDecoder->GetConstantsOffset()));
    std::vector<Constant> constants;
    constants.reserve(constantDecoder->size());
    for (uint32_t i = 0; i < constantDecoder->size(); ++i) {
        constants.emplace_back(i, constantDecoder->getConstantMrtIndex(i),
                               constantDecoder->getConstantSparsityDimension(i));
    }

    json json;
    json["header"] = header;
    json["modules"] = modules;
    json["resources"] = resources;
    json["model_sequence"] = modelSequence;
    json["constants"] = constants;

    return json;
}
