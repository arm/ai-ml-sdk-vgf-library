/*
 * SPDX-FileCopyrightText: Copyright 2025-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf-utils/temp_folder.hpp"

#include <algorithm>
#include <random>
#include <string>

namespace {
const std::filesystem::path system_temp_folder_path = std::filesystem::temp_directory_path();

std::string randomString() {
    std::string alphaNumeric("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    std::shuffle(alphaNumeric.begin(), alphaNumeric.end(), generator);
    return alphaNumeric.substr(0, 16);
}

std::filesystem::path make_non_preferred(const std::filesystem::path &path) {
    std::string pathStr = path.generic_string();
    std::replace(pathStr.begin(), pathStr.end(), '\\', '/');
    return std::filesystem::path(pathStr);
}
} // namespace

TempFolder::TempFolder(std::string_view prefix) {
    std::string pathName = std::string(prefix) + std::string("_") + randomString();
    tempFolderPath_ = system_temp_folder_path / pathName;
    tempFolderPath_ = make_non_preferred(tempFolderPath_);
    std::filesystem::create_directories(tempFolderPath_);
}

TempFolder::~TempFolder() { std::filesystem::remove_all(tempFolderPath_); }

std::filesystem::path &TempFolder::path() { return tempFolderPath_; }

std::filesystem::path TempFolder::relative(std::string_view path) const { return tempFolderPath_ / path; }
