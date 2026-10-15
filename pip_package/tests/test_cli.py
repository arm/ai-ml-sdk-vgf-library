#
# SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
import os
import sys
from pathlib import Path
from types import SimpleNamespace

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from vgf_lib import cli
from vgf_lib import updater_cli


@pytest.mark.parametrize(
    ("platform", "binary_name"),
    (("linux", "vgf_updater"), ("win32", "vgf_updater.exe")),
)
def test_binary_name_for_platform(
    platform: str,
    binary_name: str,
) -> None:
    assert cli._binary_name("vgf_updater", platform) == binary_name


def test_run_binary_forwards_arguments_and_exit_code(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    run_calls: list[list[str]] = []

    def run(command: list[str]) -> SimpleNamespace:
        run_calls.append(command)
        return SimpleNamespace(returncode=7)

    monkeypatch.setattr(cli.sys, "argv", ["vgf_updater", "--help"])
    monkeypatch.setattr(cli.subprocess, "run", run)

    assert cli.run_binary("vgf_updater") == 7
    binary_name = cli._binary_name("vgf_updater", sys.platform)
    assert run_calls == [
        [
            os.path.join(os.path.dirname(cli.__file__), "binaries", "bin", binary_name),
            "--help",
        ]
    ]


def test_updater_entry_point_runs_packaged_binary(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    names: list[str] = []

    def run_binary(name: str) -> int:
        names.append(name)
        return 3

    monkeypatch.setattr(updater_cli, "run_binary", run_binary)

    assert updater_cli.main() == 3
    assert names == ["vgf_updater"]
