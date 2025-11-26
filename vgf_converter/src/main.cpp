/*
 * SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vgf_converter.hpp"
#include <argparse/argparse.hpp>
#include <filesystem>
#include <iostream>
#include <stdexcept>

int main(int argc, char *argv[]) {
    argparse::ArgumentParser parser(argv[0]);

    // Set up positional args
    parser.add_argument("-i", "--input").help("The VGF input file to convert").required();
    parser.add_argument("-o", "--output").help("Path to updated VGF file").required();

    parser.parse_args(argc, argv);
    const std::string input = parser.get("--input");
    const std::string output = parser.get("--output");

    // Validate paths
    const std::filesystem::path inputPath{input};
    const std::filesystem::path outputPath{output};

    if (!std::filesystem::exists(inputPath)) {
        std::cerr << "File does not exist: " << input << std::endl;
        return EXIT_FAILURE;
    }

    if (!std::filesystem::is_regular_file(inputPath)) {
        std::cerr << "Input path is not a file: " << input << std::endl;
        return EXIT_FAILURE;
    }

    // Check that input and output paths are different
    if (std::filesystem::exists(outputPath) && std::filesystem::equivalent(inputPath, outputPath)) {
        std::cerr << "Input path '" << input << "' is identical to output path '" << output << "'" << std::endl;
        return EXIT_FAILURE;
    }

    try {
        mlsdk::vgf_converter::convert(input, output);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
