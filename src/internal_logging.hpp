/*
 * SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "vgf/logging.hpp"

#include <string>

namespace mlsdk::vgflib::logging {

/// \brief Log the message with log level INFO
///
/// \param message Log message
void info(const std::string &message);

/// \brief Log the message with log level WARNING
///
/// \param message Log message
void warning(const std::string &message);

/// \brief Log the message with log level DEBUG
///
/// \param message Log message
void debug(const std::string &message);

/// \brief Log the message with log level ERROR
///
/// \param message Log message
void error(const std::string &message);

} // namespace mlsdk::vgflib::logging
