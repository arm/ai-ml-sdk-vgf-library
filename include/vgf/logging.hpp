/*
 * SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <functional>
#include <ostream>
#include <string>

namespace mlsdk::vgflib::logging {

/**
 * \defgroup VGFLoggingAPI Logging API
 * @{
 */

/// \brief Enum for the log levels
enum class LogLevel {
    INFO,
    WARNING,
    DEBUG,
    ERROR,
};

/// \brief Type definition for the logging callback
///
/// \param logLevel Log level of the message
/// \param message Log message
using LoggingCallback = std::function<void(LogLevel logLevel, const std::string &message)>;

/// \brief Support LogLevel in std::ostream output operator
///
/// \param os Reference to the std::ostream instance
/// \param logLevel Log level
/// \returns Reference to the std::ostream instance
std::ostream &operator<<(std::ostream &os, const LogLevel &logLevel);

/// \brief Enables logging functionality
///
/// \param callback Callback that should be used for processing the log messages
void EnableLogging(const LoggingCallback &callback);

/// \brief Disables logging functionality
void DisableLogging();

/**@}*/
} // namespace mlsdk::vgflib::logging
