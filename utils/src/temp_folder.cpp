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
    std::string alpha_numeric("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    std::shuffle(alpha_numeric.begin(), alpha_numeric.end(), generator);
    return alpha_numeric.substr(0, 16);
}

std::filesystem::path make_non_preferred(const std::filesystem::path &path) {
    std::string path_str = path.generic_string();
    std::replace(path_str.begin(), path_str.end(), '\\', '/');
    return std::filesystem::path(path_str);
}
} // namespace

TempFolder::TempFolder(std::string_view prefix) {
    std::string path_name = std::string(prefix) + std::string("_") + randomString();
    tempFolderPath_ = system_temp_folder_path / path_name;
    tempFolderPath_ = make_non_preferred(tempFolderPath_);
    std::filesystem::create_directories(tempFolderPath_);
}

TempFolder::~TempFolder() { std::filesystem::remove_all(tempFolderPath_); }

std::filesystem::path &TempFolder::path() { return tempFolderPath_; }

std::filesystem::path TempFolder::relative(std::string_view path) const { return tempFolderPath_ / path; }
