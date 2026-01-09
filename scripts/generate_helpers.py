#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: Copyright 2024-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
import argparse
import os
import subprocess
from collections import defaultdict


def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("vulkan_header", type=str, help="Location of vulkan_core.h")
    parser.add_argument(
        "vulkan_format_traits", type=str, help="Location of vulkan_format_traits.hpp"
    )
    parser.add_argument("vulkan_enums", type=str, help="Location of vulkan_enums.hpp")
    parser.add_argument("output", type=str, help="Output (generated) header file")
    return parser.parse_args()


# Collects all the unique values of all the enumerations
def gather_unique_enum_values(vk_name, file_in):
    file_in.seek(0)
    read_enum = False
    value_to_name = dict()
    name_to_value = dict()
    if (filename := os.path.basename(file_in.name)) == "vulkan_core.h":
        check_string = f"typedef enum {vk_name}"
    elif filename == "vulkan_enums.hpp":
        check_string = f"enum class {vk_name}"
    else:
        raise ValueError(
            f"Unsupported file: {filename}. Only vulkan_core.h or vulkan_enums.hpp are accepted"
        )

    for line in file_in:
        if not read_enum and check_string in line:
            read_enum = True
        elif read_enum:
            if "}" in line:
                break
            elif "=" in line:
                name, value = [x.strip().replace(",", "") for x in line.split("=")]

                if str(value) not in name_to_value.keys():
                    value_to_name[str(value)] = name
                    name_to_value[name] = value
                else:
                    value_to_name[name_to_value[str(value)]] += " / " + name

    return value_to_name, name_to_value


def UnVkName(vkName):
    if vkName[:2] == "Vk":
        return vkName[2:]
    return vkName


# Create a comment in the code wrapped to 'line_length'
def string_to_comment(s, line_length=100):
    out = str()
    comment_prefix = "// "

    char_idx = 0
    last_line_start = 0
    last_space = 0

    # Account for comment_prefix in the line length
    line_length -= len(comment_prefix)

    # Wrap the string
    for c in s:
        if c == " ":
            # Copy the line at the last space char that preserves line_length limit
            if char_idx - last_line_start > line_length:
                out += comment_prefix + s[last_line_start:last_space].lstrip() + "\n"
                last_line_start = last_space
            last_space = char_idx

        char_idx += 1

    out += comment_prefix + s[last_line_start:].lstrip() + "\n"
    return out


# Generate a function to validate the value of an enumeration type after a cast
# This could become out-of-sync if the VGF generator used has Vulkan symbols not
# available in the VGF consumer. Simple fix is to regenerate the header with
# an up-to-date Vulkan core header.
def generate_validation_for_enum(vk_name, target_name, vk_header):
    generated = str()

    comment = (
        "Validate that the decoded enum is valid given the Vulkan headers version in use. "
        "If this call fails, it indicates that either a newer vulkan_core.h was used by the "
        "VGF generator OR that the VGF is corrupted in some other way. Regenerating this "
        "header file with a newer Vulkan headers version may resolve the issue."
    )

    generated += string_to_comment(comment)
    generated += f"inline bool ValidateDecoded{target_name}({target_name} e) " "{\n"
    generated += "switch (static_cast<int>(e)) {\n"

    value_to_name = gather_unique_enum_values(vk_name, vk_header)[0]

    for value in value_to_name:
        generated += f"case {value}: // {value_to_name[value]}\n"

    generated += f"    return true;\n"
    generated += (
        # fmt: off
        "default:\n"
        "    return false;\n"
        # fmt: on
    )

    generated += "}\n"
    generated += "}\n"
    generated += "\n"

    return generated


# Generate enum to string function
def generate_to_string_for_enum(vk_name, target_name, vk_header):
    generated = str()

    comment = f"Convert a {target_name} enum value to cstring name"
    generated += string_to_comment(comment)

    generated += f"inline std::string {target_name}ToName({target_name} e) " "{\n"
    generated += "switch (static_cast<int>(e)) {\n"

    value_to_name = gather_unique_enum_values(vk_name, vk_header)[0]

    for value in value_to_name:
        generated += f"case {value}:\n"
        generated += f'    return "{value_to_name[value]}";\n'

    generated += (
        # fmt: off
        "default: { \n"
        "        std::stringstream ss;\n"
        "        ss << \"Unknown(\" << e << \")\";\n"
        "        return ss.str();\n"
        "    }\n"
        # fmt: on
    )
    generated += "}\n"
    generated += "}\n"
    generated += "\n"

    return generated


# Generate string to enum function
def generate_to_enum_for_string(vk_name, target_name, vk_header):
    generated = str()

    comment = f"Convert a string name to {target_name}enum value"
    generated += string_to_comment(comment)

    generated += (
        f"inline {target_name} NameTo{target_name}(const std::string& str) " "{\n"
    )

    name_to_value = gather_unique_enum_values(vk_name, vk_header)[1]

    for name in name_to_value:
        generated += f'if (str == "{name}") {{\n'
        generated += f"    return {name_to_value[name]};\n"
        generated += f"}}\n"

    generated += " // Unknown name\n" " return -1;\n"
    generated += "}\n"
    generated += "\n"

    return generated


# Generate any function for a given enum
def generate_func_for_enum(vk_name, target_name, vk_header, func):
    vk_header.seek(0)
    generated = str()
    generated += func(vk_name, target_name, vk_header)
    return generated


def generate_header_version_constant(vk_header):
    vk_header.seek(0)
    generated = str()

    line = vk_header.readline()
    while line:
        if line.find(f"#define VK_HEADER_VERSION ") != -1:
            comment = f"Record the VK_HEADER_VERSION from the vulkan_core.h used to generate this file"
            generated += string_to_comment(comment)
            generated += f"const int32_t HEADER_VERSION_USED_FOR_HELPER_GENERATION = {line.split()[-1]};\n\n"

        line = vk_header.readline()

    return generated


# Generate the fake Vulkan enum type as a int32_t
def generate_enum_cast(src, target, alias=None):
    generated = str()

    if alias == None:
        alias = target
    comment = f"Cast a {src} type to a {target} type"
    generated += string_to_comment(comment)

    generated += (
        f"inline {target} To{alias}({src} e) "
        "{ "
        f"return static_cast<{target}>(e); "
        "}\n\n"
    )

    return generated


# Generate the two cast (to and from) functions between Vulkan enum and VGF representative type
def generate_enum_casts(a, b):
    generated = generate_enum_cast(a, b) + generate_enum_cast(b, a)

    # special case to generate the raii format types
    if a == "VkFormat" and b == "FormatType":
        generated += "#if defined(VULKAN_RAII_HPP)\n"
        generated += generate_enum_cast(a, "vk::Format", "RaiiFormat")
        generated += generate_enum_cast(b, "vk::Format", "RaiiFormat")
        generated += generate_enum_cast("vk::Format", a)
        generated += generate_enum_cast("vk::Format", b)
        generated += "#endif\n\n"

    generated += string_to_comment("Ensure validity of casted types")
    generated += f"static_assert(sizeof({a}) == sizeof({b}));\n\n"

    return generated


# Generate the fake Vulkan enum type as a int32_t
def generate_fake_enum_type(name):
    generated = "#if defined(VGFLIB_VK_HELPERS)\n"
    generated += f"using {name} = int32_t;\n"
    generated += "#endif\n\n"

    return generated


# Generate several helper functions for an enum matching vk_name
def generate_enum_code(vk_name, target_name, vk_header):
    return (
        generate_fake_enum_type(vk_name)
        + generate_enum_casts(vk_name, target_name)
        + generate_func_for_enum(
            vk_name, target_name, vk_header, generate_to_string_for_enum
        )
        + generate_func_for_enum(
            vk_name, target_name, vk_header, generate_to_enum_for_string
        )
        + generate_func_for_enum(
            vk_name, target_name, vk_header, generate_validation_for_enum
        )
    )


# Parse Vulkan enum format traits functions to retrieve block size and
# component numeric format from vulkan_format_traits.hpp.
def parse_numeric_format(
    vk_format_traits, vk_format_to_format_type_name, name_to_format_type
):
    vk_format_traits.seek(0)
    read_format = False
    read_component = False
    format_type_to_numeric_format = defaultdict(dict)

    for line in vk_format_traits:
        if (
            not read_format
            and "VULKAN_HPP_INLINE" in line
            and "componentNumericFormat" in line
        ):
            read_format = True
        elif read_format:
            if 'default: return "";' in line:
                read_format = False
                break
            elif not read_component and "case" in line:
                vk_enum_format = line.split("::")[-1].strip().replace(":", "")
                name = vk_format_to_format_type_name[vk_enum_format]
                format_type = name_to_format_type[name]
                read_component = True
            elif read_component:
                if "}" in line:
                    read_component = False
                elif "case" in line:
                    component, numeric = [
                        x.strip().replace("case ", "").replace(";", "")
                        for x in line.split(": return")
                    ]
                    format_type_to_numeric_format[format_type][component] = numeric

    return format_type_to_numeric_format


def parse_block_size(
    vk_format_traits, vk_format_to_format_type_name, name_to_format_type
):
    vk_format_traits.seek(0)
    read_switch = False
    format_type_to_block_size = {}

    for line in vk_format_traits:
        if not read_switch and "VULKAN_HPP_INLINE" in line and "blockSize" in line:
            read_switch = True
        if read_switch:
            if "}" in line:
                break
            elif "case" in line:
                vk_enum_format, size = [
                    x.strip().replace(";", "")
                    for x in line.split("::")[-1].split(": return")
                ]
                name = vk_format_to_format_type_name[vk_enum_format]
                format_type = name_to_format_type[name]
                format_type_to_block_size[format_type] = size

    return format_type_to_block_size


def generate_format_type_block_size(
    format_type_to_block_size, format_type_to_numeric_format
):
    comment = "Get the texel block size in bytes of the given format."
    generated = string_to_comment(comment)
    generated += f"inline uint8_t blockSize ( {'FormatType'} e ) " "{\n"
    generated += "switch (static_cast<int>(e)) {\n"

    for format_type, size in format_type_to_block_size.items():
        # Filter out unnecessary format types when dumping constants
        components = format_type_to_numeric_format[format_type]
        if "0" in components and len(components.items()) == 1:
            generated += f"case {format_type}: return {size};\n"

    generated += "\ndefault: return 0;\n"
    generated += "}\n"
    generated += "}\n"
    generated += "\n"

    return generated


def generate_format_type_component_numeric_format(format_type_to_numeric_format):
    comment = "Get the numeric format of the given format."
    generated = string_to_comment(comment)
    generated += "inline std::string componentNumericFormat (FormatType e) {\n"
    generated += "switch (static_cast<int>(e)) {\n"

    for format_type, components in format_type_to_numeric_format.items():
        # Filter out unnecessary format types when dumping constants
        if "0" in components and len(components.items()) == 1:
            generated += f"case {format_type}: return {components['0']};\n"

    generated += '\ndefault: return "";\n'
    generated += "}\n"
    generated += "}\n"
    generated += "\n"

    return generated


def generate_func_for_format_type_traits(vk_header, vk_format_traits, vk_enums):
    generated = str()
    vk_header.seek(0)

    name_to_format_type = gather_unique_enum_values("VkFormat", vk_header)[1]
    vk_format_to_format_type_name = gather_unique_enum_values("Format", vk_enums)[1]
    format_type_to_numeric_format = parse_numeric_format(
        vk_format_traits, vk_format_to_format_type_name, name_to_format_type
    )
    format_type_to_block_size = parse_block_size(
        vk_format_traits, vk_format_to_format_type_name, name_to_format_type
    )

    generated += generate_format_type_block_size(
        format_type_to_block_size, format_type_to_numeric_format
    )
    generated += generate_format_type_component_numeric_format(
        format_type_to_numeric_format
    )

    return generated


def main():
    args = parse_arguments()

    if not os.path.isfile(args.vulkan_header):
        print(f"{args.vulkan_header} does not exists")
    else:
        print(f"Found {args.vulkan_header}")

    # Generate the functions
    with open(args.vulkan_header, "r") as vkHeader:
        vkHeaderVersion = generate_header_version_constant(vkHeader)
        vkDescriptorTypeFuncs = generate_enum_code(
            "VkDescriptorType", "DescriptorType", vkHeader
        )
        vkFormatFuncs = generate_enum_code("VkFormat", "FormatType", vkHeader)
        with open(args.vulkan_format_traits, "r") as vkFormatTraits:
            with open(args.vulkan_enums, "r") as vkEnums:
                vkFormatTraitsFunctions = generate_func_for_format_type_traits(
                    vkHeader, vkFormatTraits, vkEnums
                )

    # Write the generated file based on the template
    with open("vulkan_helpers.template.hpp", "r") as template:
        with open(args.output, "w") as dst_header:
            src_line = template.readline()
            while src_line:
                if src_line.find("<GENERATED_CODE_GOES_HERE>") != -1:
                    dst_header.write(vkHeaderVersion)
                    dst_header.write(vkDescriptorTypeFuncs)
                    dst_header.write(vkFormatFuncs)
                    dst_header.write(vkFormatTraitsFunctions)
                else:
                    dst_header.write(src_line)

                src_line = template.readline()

    subprocess.call(["clang-format", "-i", args.output])
    print(f"Generated header {args.output}")


if __name__ == "__main__":
    main()
