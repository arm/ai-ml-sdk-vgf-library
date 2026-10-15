#
# SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
from .cli import run_binary


def main():
    """Run the packaged VGF updater executable."""
    return run_binary("vgf_updater")
