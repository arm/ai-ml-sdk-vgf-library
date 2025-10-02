#
# SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
from datetime import datetime

from setuptools import find_packages
from setuptools import setup

setup(
    name="ai_ml_sdk_vgf_library",
    version=datetime.today().strftime("%m.%d"),
    packages=find_packages(),
    include_package_data=True,
    package_data={
        "vgf_lib": ["binaries/*", "binaries/*/*", "binaries/*/*/*"],
    },
    entry_points={
        "console_scripts": [
            "vgf_dump=vgf_lib.cli:main",
        ]
    },
    zip_safe=False,
)
