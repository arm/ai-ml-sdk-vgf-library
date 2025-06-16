#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: Copyright 2024 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
import sys
from pathlib import Path

import pytest


def valid_dir(value):
    """Check if provided value is a valid path to a directory."""
    path = Path(value)

    if not path.is_dir():
        raise pytest.UsageError(f"Path {path} does not exist or not a directory")

    return path.resolve().as_posix()


# Add command line options
def pytest_addoption(parser):
    parser.addoption(
        "--vgf-pylib-dir",
        action="store",
        type=valid_dir,
        required=True,
        help="Directory of VGF Python Lib file",
    )


def pytest_configure(config):
    sys.path.append(config.option.vgf_pylib_dir)
