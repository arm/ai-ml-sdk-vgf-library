#
# SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
import os
import pathlib
import platform
import shutil
import subprocess
import sys

ROOT_DIR = pathlib.Path(__file__).resolve().parents[1]
DEPENDENCY_DIR = (ROOT_DIR / ".." / ".." / "dependencies").resolve()
CMAKE_TOOLCHAIN_PATH = ROOT_DIR / "cmake" / "toolchain"


def build_native(extension_output_path, install_dir=None):
    extension_output_path = pathlib.Path(extension_output_path).resolve()
    if install_dir is not None:
        install_dir = pathlib.Path(install_dir).resolve()
    _configure_and_build(extension_output_path, install_dir)


def _configure_and_build(extension_output_path, install_dir=None):
    build_dir = pathlib.Path(
        os.environ.get("VGF_PIP_BUILD_DIR", ROOT_DIR / "build" / "pip")
    ).resolve()
    setuptools_build_dir = extension_output_path.parent
    if install_dir is None:
        install_dir = setuptools_build_dir / "vgf_lib" / "binaries"
    build_type = os.environ.get("VGF_PIP_BUILD_TYPE", "Release")
    generator = os.environ.get("CMAKE_GENERATOR", "Ninja")

    shutil.rmtree(install_dir, ignore_errors=True)
    setuptools_build_dir.mkdir(parents=True, exist_ok=True)

    cmake_setup_cmd = [
        "cmake",
        "-S",
        str(ROOT_DIR),
        "-B",
        str(build_dir),
        f"-DCMAKE_BUILD_TYPE={build_type}",
        "-G",
        generator,
        "-DML_SDK_VGF_LIB_BUILD_PYLIB=ON",
        f"-DPYTHON_EXECUTABLE={sys.executable}",
        f"-DPython_EXECUTABLE={sys.executable}",
        f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={setuptools_build_dir}",
        f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{build_type.upper()}={setuptools_build_dir}",
        f"-DARGPARSE_PATH={_env_path('ARGPARSE_PATH', DEPENDENCY_DIR / 'argparse')}",
        f"-DJSON_PATH={_env_path('JSON_PATH', DEPENDENCY_DIR / 'json')}",
        f"-DFLATBUFFERS_PATH={_env_path('FLATBUFFERS_PATH', DEPENDENCY_DIR / 'flatbuffers')}",
    ]
    cmake_setup_cmd.extend(_pybind11_args())
    cmake_setup_cmd.extend(_host_toolchain_args())
    cmake_prefix_path = os.environ.get("CMAKE_PREFIX_PATH")
    if cmake_prefix_path:
        cmake_setup_cmd.append(f"-DCMAKE_PREFIX_PATH={cmake_prefix_path}")
    package_version = os.environ.get("ML_SDK_PACKAGE_VERSION")
    if package_version:
        cmake_setup_cmd.append(f"-DML_SDK_PACKAGE_VERSION={package_version}")

    cmake_build_cmd = [
        "cmake",
        "--build",
        str(build_dir),
        "--config",
        build_type,
        "--parallel",
        os.environ.get("CMAKE_BUILD_PARALLEL_LEVEL", str(os.cpu_count() or 1)),
    ]
    cmake_install_cmd = [
        "cmake",
        "--install",
        str(build_dir),
        "--prefix",
        str(install_dir),
        "--config",
        build_type,
    ]

    subprocess.run(cmake_setup_cmd, check=True)
    subprocess.run(cmake_build_cmd, check=True)
    subprocess.run(cmake_install_cmd, check=True)

    if not extension_output_path.is_file():
        raise RuntimeError(
            f"CMake did not write the extension to {extension_output_path}"
        )


def _env_path(name, default):
    return pathlib.Path(os.environ.get(name, default)).resolve()


def _pybind11_args():
    pybind11_path = _env_path("PYBIND11_PATH", DEPENDENCY_DIR / "pybind11")
    if pybind11_path.exists():
        return [f"-DPYBIND11_PATH={pybind11_path}"]

    try:
        import pybind11
    except ImportError:
        return [f"-DPYBIND11_PATH={pybind11_path}"]

    return [
        f"-DPYBIND11_PATH={pybind11_path}",
        f"-Dpybind11_DIR={pybind11.get_cmake_dir()}",
    ]


def _host_toolchain_args():
    system = platform.system()
    if system == "Linux":
        return [f"-DCMAKE_TOOLCHAIN_FILE={CMAKE_TOOLCHAIN_PATH / 'gcc.cmake'}"]
    if system == "Darwin":
        return [f"-DCMAKE_TOOLCHAIN_FILE={CMAKE_TOOLCHAIN_PATH / 'clang.cmake'}"]
    if system == "Windows":
        return [
            f"-DCMAKE_TOOLCHAIN_FILE={CMAKE_TOOLCHAIN_PATH / 'windows-msvc.cmake'}",
            "-DMSVC=ON",
        ]

    raise RuntimeError(f"Unsupported host platform: {system}")
