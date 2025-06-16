/*
 * SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <argparse/argparse.hpp>

#include <version.hpp>
#include <vgf/vgf_dump.hpp>

#include <filesystem>

using namespace mlsdk::vgf_dump;

int main(int argc, char *argv[]) {
    try {
        argparse::ArgumentParser parser(argv[0], details::version);
        auto &outputGroup = parser.add_mutually_exclusive_group();
        parser.set_usage_max_line_width(80);

        parser.add_argument("-i", "--input").help("The VGF input file").required();
        parser.add_argument("-o", "--output").help("The output file").default_value<std::string>("-").required();
        outputGroup.add_argument("--dump-spirv")
            .help("Dump the SPIR-V™ module code at the given index")
            .scan<'i', int>();
        outputGroup.add_argument("--dump-constant")
            .help("Dump the constant at the given index. Outputs NumPy file if output file is .npy, "
                  "otherwise dumps as raw binary.")
            .scan<'i', int>();
        outputGroup.add_argument("--scenario-template")
            .help("Create a scenario template based on the VGF")
            .default_value(false)
            .implicit_value(true);
        parser.add_argument("--scenario-template-add-boundaries")
            .help("If creating a scenario template, add frame boundaries before and after")
            .default_value(false)
            .implicit_value(true);

        parser.parse_args(argc, argv);

        std::string input = parser.get("--input");
        std::string output = parser.get("--output");

        if (parser.present<int>("--dump-spirv")) {
            dumpSpirv(input, output, uint32_t(parser.get<int>("--dump-spirv")));
        } else if (parser.present<int>("--dump-constant")) {
            if (std::filesystem::path(output).extension() == ".npy") {
                dumpNumpy(input, output, uint32_t(parser.get<int>("--dump-constant")));
            } else {
                dumpConstant(input, output, uint32_t(parser.get<int>("--dump-constant")));
            }
        } else if (parser.get<bool>("--scenario-template")) {
            dumpScenario(input, output, parser.get<bool>("--scenario-template-add-boundaries"));
        } else {
            dumpFile(input, output);
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
