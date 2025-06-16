/*
 * SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf/logging.h"

#include "internal_logging.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <sstream>
#include <utility>
#include <vector>

using namespace mlsdk::vgflib::logging;

using LogEntry = std::pair<LogLevel, std::string>;
using LogEntryC = std::pair<mlsdk_logging_log_level, std::string>;

TEST(CppLogging, EnableDisable) {
    std::vector<LogEntry> entries;
    auto callback = [&entries](LogLevel logLevel, const std::string &message) {
        entries.emplace_back(logLevel, message);
    };

    EnableLogging(callback);
    info("Info message");
    debug("Debug message");
    warning("Warning message");
    error("Error message");

    DisableLogging();
    info("Another test message");

    std::vector<LogEntry> expected = {{LogLevel::INFO, "Info message"},
                                      {LogLevel::DEBUG, "Debug message"},
                                      {LogLevel::WARNING, "Warning message"},
                                      {LogLevel::ERROR, "Error message"}};
    ASSERT_TRUE(std::equal(entries.begin(), entries.end(), expected.begin(), expected.end()));
}

TEST(CppLogging, StringConversion) {
    auto asStr = [](LogLevel logLevel) {
        std::stringstream output;
        output << logLevel;
        return output.str();
    };

    ASSERT_TRUE(asStr(LogLevel::INFO) == "INFO");
    ASSERT_TRUE(asStr(LogLevel::DEBUG) == "DEBUG");
    ASSERT_TRUE(asStr(LogLevel::WARNING) == "WARNING");
    ASSERT_TRUE(asStr(LogLevel::ERROR) == "ERROR");
}

namespace {
std::vector<LogEntryC> entries;
void loggingCallback(mlsdk_logging_log_level logLevel, const char *message) { entries.emplace_back(logLevel, message); }
} // namespace

TEST(CLogging, EnableDisable) {
    mlsdk_logging_enable(loggingCallback);
    info("Info message");
    debug("Debug message");
    warning("Warning message");
    error("Error message");

    mlsdk_logging_disable();
    info("Another test message");

    std::vector<LogEntryC> expected = {{mlsdk_logging_log_level_info, "Info message"},
                                       {mlsdk_logging_log_level_debug, "Debug message"},
                                       {mlsdk_logging_log_level_warning, "Warning message"},
                                       {mlsdk_logging_log_level_error, "Error message"}};
    ASSERT_TRUE(std::equal(entries.begin(), entries.end(), expected.begin(), expected.end()));
}
