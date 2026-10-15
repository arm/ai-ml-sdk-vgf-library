#
# SPDX-FileCopyrightText: Copyright 2025-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
import os
import subprocess
import sys


def _binary_name(binary_stem, platform):
    return f"{binary_stem}.exe" if platform.startswith("win") else binary_stem


def run_binary(binary_stem):
    binary_name = _binary_name(binary_stem, sys.platform)
    binary_path = os.path.join(
        os.path.dirname(__file__), "binaries", "bin", binary_name
    )
    result = subprocess.run([binary_path] + sys.argv[1:])
    return result.returncode


def main():
    return run_binary("vgf_dump")
