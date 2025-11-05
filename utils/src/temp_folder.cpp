/*
 * SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
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

std::filesystem::path make_non_preferred(std::filesystem::path path) {
    std::string path_str = path.generic_string();
    std::replace(path_str.begin(), path_str.end(), '\\', '/');
    return std::filesystem::path(path_str);
}
} // namespace

TempFolder::TempFolder(std::string_view prefix) {
    std::string path_name = std::string(prefix) + std::string("_") + randomString();
    temp_folder_path = system_temp_folder_path / path_name;
    temp_folder_path = make_non_preferred(temp_folder_path);
    std::filesystem::create_directories(temp_folder_path);
}

TempFolder::~TempFolder() { std::filesystem::remove_all(temp_folder_path); }

std::filesystem::path &TempFolder::path() { return temp_folder_path; }

std::filesystem::path TempFolder::relative(std::string_view path) const { return temp_folder_path / path; }
