#
# SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

import build_native


def test_build_native_limits_build_and_install_to_wheel_payload(
    monkeypatch: pytest.MonkeyPatch,
    tmp_path: Path,
) -> None:
    extension = tmp_path / "setuptools" / "vgfpy.so"
    commands: list[list[str]] = []

    def run(command: list[str], check: bool) -> None:
        assert check
        commands.append(command)
        if "--build" in command:
            extension.parent.mkdir(parents=True, exist_ok=True)
            extension.touch()

    monkeypatch.setattr(build_native.subprocess, "run", run)
    monkeypatch.setattr(build_native.shutil, "which", lambda _: "/usr/bin/ninja")

    build_native.build_native(extension)

    configure_command = commands[0]
    assert configure_command[configure_command.index("-B") + 1] == str(
        build_native.ROOT_DIR / "build" / "pip" / build_native.PYTHON_ABI_TAG
    )

    assert "-DCMAKE_MAKE_PROGRAM=/usr/bin/ninja" in configure_command

    build_command = commands[1]
    assert build_command[build_command.index("--target") + 1 : -2] == [
        "vgfpy",
        "vgf_dump",
        "vgf_updater",
    ]
    install_command = commands[2]
    assert install_command[install_command.index("--component") + 1] == "mlsdk_wheel"


def test_build_native_rejects_staged_mode(
    monkeypatch: pytest.MonkeyPatch,
    tmp_path: Path,
) -> None:
    monkeypatch.setenv("MLSDK_WHEEL_BUILD_MODE", "staged")

    with pytest.raises(RuntimeError, match="require source mode"):
        build_native.build_native(tmp_path / "vgfpy.so")
