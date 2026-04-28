/*
 * SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "vgf-utils/temp_folder.hpp"
#include "vgf/encoder.h"
#include "vgf/logging.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

namespace mlsdk::vgflib::logging::utils {

class Logger {
  public:
    Logger() {
        EnableLogging([this](LogLevel, const std::string &message) { messages_.push_back(message); });
    }

    ~Logger() { DisableLogging(); }

    bool contains(std::initializer_list<std::string_view> tokens) const {
        for (const auto &msg : messages_) {
            bool allFound = true;
            for (const auto &token : tokens) {
                if (msg.find(token) == std::string::npos) {
                    allFound = false;
                    break;
                }
            }
            if (allFound) {
                return true;
            }
        }
        return false;
    }

  private:
    std::vector<std::string> messages_;
};

} // namespace mlsdk::vgflib::logging::utils

namespace mlsdk::vgflib::testutils {

inline std::string ReadFile(const std::filesystem::path &path) {
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

inline std::string FinishAndWriteCEncoder(mlsdk_encoder *encoder) {
    assert(encoder != nullptr && "encoder is null");
    TempFolder tempFolder("vgf_c_encoder_test");
    const std::filesystem::path path = tempFolder.relative("model.vgf");
    mlsdk_encoder_finish(encoder);
    const bool ok = mlsdk_encoder_write_to_file(encoder, path.string().c_str());
    mlsdk_encoder_destroy(encoder);
    return ok ? ReadFile(path) : std::string{};
}

} // namespace mlsdk::vgflib::testutils
