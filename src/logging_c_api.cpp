/*
 * SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf/logging.h"

#include "internal_logging.hpp"

using namespace mlsdk::vgflib::logging;

namespace {
void noLogging(mlsdk_logging_log_level, const char *) {}
mlsdk_logging_callback callback = noLogging;

void callbackWrapper(LogLevel logLevel, const std::string &message) {
    switch (logLevel) {
    case (LogLevel::INFO):
        callback(mlsdk_logging_log_level_info, message.c_str());
        break;
    case (LogLevel::DEBUG):
        callback(mlsdk_logging_log_level_debug, message.c_str());
        break;
    case (LogLevel::WARNING):
        callback(mlsdk_logging_log_level_warning, message.c_str());
        break;
    case (LogLevel::ERROR):
        callback(mlsdk_logging_log_level_error, message.c_str());
        break;
    }
}
} // namespace

void mlsdk_logging_enable(mlsdk_logging_callback newCallback) {
    callback = newCallback;
    EnableLogging(callbackWrapper);
}

void mlsdk_logging_disable() { DisableLogging(); }
