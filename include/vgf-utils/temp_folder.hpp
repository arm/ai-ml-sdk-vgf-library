/*
 * SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <filesystem>
#include <string_view>

class TempFolder {
  public:
    explicit TempFolder(std::string_view prefix);
    ~TempFolder();

    std::filesystem::path &path();
    std::filesystem::path relative(std::string_view path) const;

  private:
    std::filesystem::path temp_folder_path;
};
