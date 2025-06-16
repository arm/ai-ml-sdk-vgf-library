/*
 * SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <functional>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace mlsdk::vgf_dump {

void dumpSpirv(const std::string &inputFile, const std::string &outputFile, uint32_t index);

void dumpConstant(const std::string &inputFile, const std::string &outputFile, uint32_t index);

void dumpNumpy(const std::string &inputFile, const std::string &outputFile, uint32_t index);

void dumpScenario(const std::string &inputFile, const std::string &outputFile, bool add_boundaries);

void dumpFile(const std::string &inputFile, const std::string &outputFile);

void getSpirv(const std::string &inputFile, uint32_t index, std::function<void(const uint32_t *, size_t)> callback);

void getConstant(const std::string &inputFile, uint32_t index, std::function<void(const uint8_t *, size_t)> callback);

nlohmann::json getScenario(const std::string &inputFile, bool add_boundaries);

nlohmann::json getFile(const std::string &inputFile);

} // namespace mlsdk::vgf_dump
