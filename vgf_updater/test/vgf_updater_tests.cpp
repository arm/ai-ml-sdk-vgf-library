/*
 * SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf-utils/temp_folder.hpp"
#include "vgf_updater.hpp"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {
::testing::AssertionResult compareFiles(fs::path fileOne, fs::path fileTwo) {
    const auto fileOneSize = fs::file_size(fileOne);
    const auto fileTwoSize = fs::file_size(fileTwo);
    if (fileOneSize != fileTwoSize) {
        return testing::AssertionFailure()
               << fileOne << " and " << fileTwo << " are not equal as determined by different size";
    }

    std::ifstream fileOneBytes;
    std::ifstream fileTwoBytes;

    fileOneBytes.open(fileOne, std::ios::binary);
    fileTwoBytes.open(fileTwo, std::ios::binary);

    if (fileOneBytes.fail()) {
        return testing::AssertionFailure() << " Failed to open file " << fileOne;
    }

    if (fileTwoBytes.fail()) {
        return testing::AssertionFailure() << " Failed to open file " << fileTwo;
    }

    fileOneBytes.seekg(0, std::ios::beg);
    fileTwoBytes.seekg(0, std::ios::beg);

    std::vector<char> fileOneBuffer(fileOneSize);
    fileOneBytes.read(fileOneBuffer.data(), static_cast<std::streamsize>(fileOneSize));

    std::vector<char> fileTwoBuffer(fileTwoSize);
    fileTwoBytes.read(fileTwoBuffer.data(), static_cast<std::streamsize>(fileTwoSize));

    return std::equal(std::istreambuf_iterator<char>(fileOneBytes.rdbuf()), std::istreambuf_iterator<char>(),
                      std::istreambuf_iterator<char>(fileTwoBytes.rdbuf()))
               ? testing::AssertionSuccess()
               : testing::AssertionFailure() << "Compared files " << fileOne << " and " << fileTwo << " are different.";
}
} // namespace

class VGFUpdaterTest : public ::testing::Test {
  protected:
    const fs::path dataDirectory = TEST_DATA_DIR;
    const fs::path singleMaxpoolVgfOutdated = dataDirectory / "single_maxpool_graph_pre_v0_4_0.vgf";
    const fs::path singleMaxpoolVgfLatest = dataDirectory / "single_maxpool_graph_v0_4_0.vgf";

    const fs::path simpleConv2dVgfOutdated = dataDirectory / "simple_conv2d_rescale_graph_outdated.vgf";
    const fs::path simpleConv2dVgfLatest = dataDirectory / "simple_conv2d_rescale_graph.vgf";
};

TEST_F(VGFUpdaterTest, fileOfLatestVersion) {

    ASSERT_TRUE(fs::exists(singleMaxpoolVgfLatest));

    TempFolder tempFolder("fileOfLatestVersion");
    const fs::path outputPath = tempFolder.relative("single_maxpool_graph_noconversion.vgf");

    testing::internal::CaptureStdout();
    mlsdk::vgf_updater::update(singleMaxpoolVgfLatest.string(), outputPath.string());
    const auto out = testing::internal::GetCapturedStdout();
    EXPECT_FALSE(fs::exists(outputPath));
    EXPECT_EQ(out, "VGF file is already at the latest version: 0.4.0\n");
}

TEST_F(VGFUpdaterTest, simpleMaxpoolGraph) {

    ASSERT_TRUE(fs::exists(singleMaxpoolVgfOutdated));
    ASSERT_TRUE(fs::exists(singleMaxpoolVgfLatest));

    TempFolder tempFolder("simpleMaxpoolGraph");
    const fs::path outputPath = tempFolder.relative("single_maxpool_graph_output.vgf");

    ASSERT_NO_THROW({ mlsdk::vgf_updater::update(singleMaxpoolVgfOutdated.string(), outputPath.string()); });
    EXPECT_TRUE(fs::exists(outputPath));

    EXPECT_TRUE(compareFiles(singleMaxpoolVgfLatest, outputPath));
}

TEST_F(VGFUpdaterTest, graphWithConstants) {

    ASSERT_TRUE(fs::exists(simpleConv2dVgfOutdated));
    ASSERT_TRUE(fs::exists(simpleConv2dVgfLatest));

    TempFolder tempFolder("graphWithConstants");
    const fs::path outputPath = tempFolder.relative("simple_conv2d_output.vgf");

    ASSERT_NO_THROW({ mlsdk::vgf_updater::update(simpleConv2dVgfOutdated.string(), outputPath.string()); });
    EXPECT_TRUE(fs::exists(outputPath));

    EXPECT_TRUE(compareFiles(simpleConv2dVgfLatest, outputPath));
}
