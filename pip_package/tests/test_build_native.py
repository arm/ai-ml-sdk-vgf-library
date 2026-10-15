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
        build_native.ROOT_DIR
        / "build"
        / "pip"
        / f"{build_native.platform.system().lower()}-{build_native.platform.machine().lower()}"
        / "release"
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


def test_build_native_reuses_native_tree_across_python_abis(
    monkeypatch: pytest.MonkeyPatch,
    tmp_path: Path,
) -> None:
    commands: list[list[str]] = []

    def run(command: list[str], check: bool) -> None:
        assert check
        commands.append(command)

    monkeypatch.setattr(build_native.subprocess, "run", run)
    monkeypatch.setenv("VGF_PIP_BUILD_DIR", str(tmp_path / "native"))
    monkeypatch.setenv("AUDITWHEEL_PLAT", "manylinux_2_28_x86_64")
    monkeypatch.setenv("VGF_PIP_BUILD_TYPE", "Debug")

    for abi in ("cpython-310-x86_64-linux-gnu", "cpython-314-x86_64-linux-gnu"):
        extension = tmp_path / abi / "vgfpy.so"
        extension.parent.mkdir()
        extension.touch()
        monkeypatch.setattr(build_native, "PYTHON_ABI_TAG", abi)
        monkeypatch.setattr(
            build_native.sys, "executable", str(tmp_path / abi / "python")
        )

        build_native.build_native(extension)

        configure = commands[-3]
        assert configure[configure.index("-B") + 1] == str(
            tmp_path / "native" / "manylinux_2_28_x86_64" / "debug"
        )
        assert f"-DMLSDK_WHEEL_PYTHON_ABI={abi}" in configure
        assert f"-DPython_EXECUTABLE={tmp_path / abi / 'python'}" in configure
        assert "-UPython_*" in configure
        assert "-U_Python_*" in configure
        assert "-UPYTHON_*" in configure
        assert "-DPYBIND11_FINDPYTHON=ON" in configure


def test_build_native_rejects_staged_mode(
    monkeypatch: pytest.MonkeyPatch,
    tmp_path: Path,
) -> None:
    monkeypatch.setenv("MLSDK_WHEEL_BUILD_MODE", "staged")

    with pytest.raises(RuntimeError, match="require source mode"):
        build_native.build_native(tmp_path / "vgfpy.so")
