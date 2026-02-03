/*
 * SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "vgf/logging.hpp"

#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

namespace mlsdk::vgflib::logging::utils {

class Logger {
  public:
    Logger() {
        EnableLogging([this](LogLevel, const std::string &message) { messages.push_back(message); });
    }

    ~Logger() { DisableLogging(); }

    bool contains(std::initializer_list<std::string_view> tokens) const {
        for (const auto &msg : messages) {
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
    std::vector<std::string> messages;
};

} // namespace mlsdk::vgflib::logging::utils
