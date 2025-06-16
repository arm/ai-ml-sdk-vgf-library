/*
 * SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "internal_logging.hpp"

namespace mlsdk::vgflib::logging {

namespace {
void noLogging(LogLevel, const std::string &) {}

LoggingCallback vgflibLoggingCallback = noLogging;

void log(LogLevel logLevel, const std::string &message) { vgflibLoggingCallback(logLevel, message); }
} // namespace

void EnableLogging(const LoggingCallback &callback) { vgflibLoggingCallback = callback; }
void DisableLogging() { vgflibLoggingCallback = noLogging; }

std::ostream &operator<<(std::ostream &os, const LogLevel &logLevel) {
    switch (logLevel) {
    case LogLevel::INFO:
        os << "INFO";
        break;
    case LogLevel::WARNING:
        os << "WARNING";
        break;
    case LogLevel::DEBUG:
        os << "DEBUG";
        break;
    case LogLevel::ERROR:
        os << "ERROR";
        break;
    }

    return os;
}

void info(const std::string &message) { log(LogLevel::INFO, message); }

void warning(const std::string &message) { log(LogLevel::WARNING, message); }

void debug(const std::string &message) { log(LogLevel::DEBUG, message); }

void error(const std::string &message) { log(LogLevel::ERROR, message); }

} // namespace mlsdk::vgflib::logging
