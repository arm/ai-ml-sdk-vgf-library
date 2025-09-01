#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
import argparse
import pathlib
import platform
import subprocess
import sys

try:
    import argcomplete
except:
    argcomplete = None

VGF_LIB_DIR = pathlib.Path(__file__).resolve().parent / ".."
DEPENDENCY_DIR = VGF_LIB_DIR / ".." / ".." / "dependencies"
CMAKE_TOOLCHAIN_PATH = VGF_LIB_DIR / "cmake" / "toolchain"


class Builder:
    """
    A  class that builds the VGF Library.

    Parameters
    ----------
    args : 'dict'
        Dictionary with arguments to build the VGF Library.
    """

    def __init__(self, args) -> None:
        self.build_dir = args.build_dir
        self.test_dir = pathlib.Path(self.build_dir) / "test"
        self.prefix_path = args.prefix_path

        self.threads = args.threads
        self.run_tests = args.test
        self.run_linting = args.lint
        self.build_type = args.build_type
        self.doc = args.doc
        self.target_platform = args.target_platform
        self.cmake_toolchain_for_android = args.cmake_toolchain_for_android
        self.flatc_path = args.flatc_path
        self.argparse_path = args.argparse_path
        self.json_path = args.json_path
        self.gtest_path = args.gtest_path
        self.flatbuffers_path = args.flatbuffers_path
        self.pybind11_path = args.pybind11_path
        self.build_pylib = args.build_pylib or args.test or args.package_type == "pip"
        self.enable_gcc_sanitizers = args.enable_gcc_sanitizers
        self.install = args.install
        self.package = args.package
        self.package_type = args.package_type
        self.package_source = args.package_source

        if not self.install and self.package_type == "pip":
            self.install = "pip_install"

    def setup_platform_build(self, cmake_cmd):
        system = platform.system()
        if self.target_platform == "host":
            if system == "Linux":
                cmake_cmd.append(
                    f"-DCMAKE_TOOLCHAIN_FILE={CMAKE_TOOLCHAIN_PATH / 'gcc.cmake'}"
                )
                return True

            if system == "Darwin":
                cmake_cmd.append(
                    f"-DCMAKE_TOOLCHAIN_FILE={CMAKE_TOOLCHAIN_PATH / 'clang.cmake'}"
                )
                return True

            if system == "Windows":
                cmake_cmd.append(
                    f"-DCMAKE_TOOLCHAIN_FILE={CMAKE_TOOLCHAIN_PATH / 'windows-msvc.cmake'}"
                )
                cmake_cmd.append("-DMSVC=ON")
                return True

            print(f"ERROR: Unsupported host platform {system}", file=sys.stderr)
            return False

        if self.target_platform == "linux-clang":
            if system != "Linux":
                print(
                    f"ERROR: target {self.target_platform} only supported on Linux. Host platform {system}",
                    file=sys.stderr,
                )
                return False
            cmake_cmd.append(
                f"-DCMAKE_TOOLCHAIN_FILE={CMAKE_TOOLCHAIN_PATH / 'clang.cmake'}"
            )
            return True

        if self.target_platform == "aarch64":
            if system != "Linux":
                print(
                    f"ERROR: target {self.target_platform} only supported on Linux. Host platform {system}",
                    file=sys.stderr,
                )
                return False
            cmake_cmd.append(
                f"-DCMAKE_TOOLCHAIN_FILE={CMAKE_TOOLCHAIN_PATH / 'linux-aarch64-gcc.cmake'}"
            )
            cmake_cmd.append("-DHAVE_CLONEFILE=0")
            cmake_cmd.append("-DBUILD_TOOLS=OFF")
            cmake_cmd.append("-DBUILD_REGRESS=OFF")
            cmake_cmd.append("-DBUILD_EXAMPLES=OFF")
            cmake_cmd.append("-DBUILD_DOC=OFF")

            cmake_cmd.append("-DBUILD_WSI_WAYLAND_SUPPORT=OFF")
            cmake_cmd.append("-DBUILD_WSI_XLIB_SUPPORT=OFF")
            cmake_cmd.append("-DBUILD_WSI_XCB_SUPPORT=OFF")
            return True

        if self.target_platform == "android":
            if system != "Linux":
                print(
                    f"ERROR: target {self.target_platform} only supported on Linux. Host platform {system}",
                    file=sys.stderr,
                )
                return False
            print(
                "WARNING: Cross-compiling VGF Library for Android is currently an experimental feature."
            )
            if not self.cmake_toolchain_for_android:
                print(
                    "ERROR: No toolchain path specified for Android cross-compilation",
                    file=sys.stderr,
                )
                return False

            cmake_cmd.append(
                f"-DCMAKE_TOOLCHAIN_FILE={self.cmake_toolchain_for_android}"
            )
            cmake_cmd.append("-DANDROID_ABI=arm64-v8a")
            cmake_cmd.append("-DANDROID_PLATFORM=android-21")
            cmake_cmd.append("-DANDROID_ALLOW_UNDEFINED_SYMBOLS=ON")
            return True

        print(
            f"ERROR: Incorrect target platform option: {self.target_platform}",
            file=sys.stderr,
        )
        return False

    def run(self):
        cmake_setup_cmd = [
            "cmake",
            "-S",
            str(VGF_LIB_DIR),
            "-B",
            self.build_dir,
            f"-DCMAKE_BUILD_TYPE={self.build_type}",
        ]
        if self.prefix_path:
            cmake_setup_cmd.append(f"-DCMAKE_PREFIX_PATH={self.prefix_path}")

        if self.flatc_path:
            cmake_setup_cmd.append(f"-DFLATC_PATH={self.flatc_path}")

        if self.run_tests:
            cmake_setup_cmd.append("-DML_SDK_VGF_LIB_BUILD_TESTS=ON")

        if self.run_linting and self.target_platform != "aarch64":
            cmake_setup_cmd.append("-DML_SDK_VGF_LIB_ENABLE_LINT=ON")

        if self.doc:
            cmake_setup_cmd.append("-DML_SDK_VGF_LIB_BUILD_DOCS=ON")

        cmake_setup_cmd.append(f"-DARGPARSE_PATH={self.argparse_path}")
        cmake_setup_cmd.append(f"-DJSON_PATH={self.json_path}")
        cmake_setup_cmd.append(f"-DGTEST_PATH={self.gtest_path}")
        cmake_setup_cmd.append(f"-DFLATBUFFERS_PATH={self.flatbuffers_path}")

        if self.build_pylib:
            cmake_setup_cmd.append("-DML_SDK_VGF_LIB_BUILD_PYLIB=ON")
            cmake_setup_cmd.append(f"-DPYBIND11_PATH={self.pybind11_path}")

        if self.enable_gcc_sanitizers:
            cmake_setup_cmd.append("-DML_SDK_VGF_LIB_GCC_SANITIZERS=ON")

        if not self.setup_platform_build(cmake_setup_cmd):
            return 1

        cmake_build_cmd = [
            "cmake",
            "--build",
            self.build_dir,
            "-j",
            str(self.threads),
            "--config",
            self.build_type,
        ]

        try:
            subprocess.run(cmake_setup_cmd, check=True)
            subprocess.run(cmake_build_cmd, check=True)

            if self.run_tests:
                test_cmd = [
                    "ctest",
                    "--test-dir",
                    str(self.test_dir),
                    "-j",
                    str(self.threads),
                    "--output-on-failure",
                ]
                subprocess.run(test_cmd, check=True)

                build_type_dir = (
                    self.build_type if platform.system() == "Windows" else ""
                )

                pytest_cmd = [
                    sys.executable,
                    "-m",
                    "pytest",
                    "-n",
                    str(self.threads),
                    "test",
                    "--vgf-pylib-dir",
                    f"{self.build_dir}/src/{build_type_dir}",
                ]
                subprocess.run(pytest_cmd, cwd=VGF_LIB_DIR, check=True)

            if self.install:
                cmake_install_cmd = [
                    "cmake",
                    "--install",
                    self.build_dir,
                    "--prefix",
                    self.install,
                    "--config",
                    self.build_type,
                ]
                subprocess.run(cmake_install_cmd, check=True)

            if self.package and self.package_type != "pip":
                package_type = self.package_type or "tgz"
                cpack_generator = package_type.upper()

                cmake_package_cmd = [
                    "cpack",
                    "--config",
                    f"{self.build_dir}/CPackConfig.cmake",
                    "-C",
                    self.build_type,
                    "-G",
                    cpack_generator,
                    "-B",
                    self.package,
                    "-D",
                    "CPACK_INCLUDE_TOPLEVEL_DIRECTORY=OFF",
                ]
                subprocess.run(cmake_package_cmd, check=True)

            if self.package_type == "pip":
                subprocess.run(
                    [
                        "mkdir",
                        "-p",
                        "pip_package/vgf_lib/binaries",
                    ]
                )
                subprocess.run(
                    ["cp", "-R", f"{self.install}/.", "pip_package/vgf_lib/binaries/"]
                )
                subprocess.run(
                    [
                        "cp",
                        "-R",
                        f"{self.build_dir}/src/.",
                        "pip_package/vgf_lib/binaries/",
                    ]
                )
                result = subprocess.Popen(
                    [
                        "python",
                        "setup.py",
                        "bdist_wheel",
                        "--plat-name",
                        "manyLinux2014_x86_64",
                    ],
                    cwd="pip_package",
                )
                result.communicate()
                if result.returncode != 0:
                    print("ERROR: Failed to generate pip package")
                    return 1

            if self.package_source:
                package_type = self.package_type or "tgz"
                cpack_generator = package_type.upper()

                cmake_package_cmd = [
                    "cpack",
                    "--config",
                    f"{self.build_dir}/CPackSourceConfig.cmake",
                    "-C",
                    self.build_type,
                    "-G",
                    cpack_generator,
                    "-B",
                    self.package_source,
                    "-D",
                    "CPACK_INCLUDE_TOPLEVEL_DIRECTORY=OFF",
                ]
                subprocess.run(cmake_package_cmd, check=True)
        except Exception as e:
            print(f"ERROR: Build failed with error: {e}", file=sys.stderr)
            return 1

        return 0


def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--build-dir",
        help="Name of folder where to build. Default: build",
        default=f"{VGF_LIB_DIR / 'build'}",
    )
    parser.add_argument(
        "--threads",
        "-j",
        type=int,
        help="Number of threads to use for building. Default: %(default)s",
        default=16,
    )
    parser.add_argument(
        "--prefix-path",
        help="Path to prefix directory.",
    )
    parser.add_argument(
        "-t",
        "--test",
        help="Run unit tests after build. Default: %(default)s",
        action="store_true",
        default=False,
    )
    parser.add_argument(
        "-l",
        "--lint",
        help="Run linter. Default: %(default)s",
        action="store_true",
        default=False,
    )
    parser.add_argument(
        "--build-type",
        help="Type of build to perform. Default: %(default)s",
        default="Release",
    )
    parser.add_argument(
        "--flatc-path",
        help="Path to the flatc compiler",
        default="",
    )
    parser.add_argument(
        "--doc",
        help="Build documentation. Default: %(default)s",
        action="store_true",
        default=False,
    )
    parser.add_argument(
        "--target-platform",
        help="Specify the target build platform",
        choices=["host", "android", "aarch64", "linux-clang"],
        default="host",
    )
    parser.add_argument(
        "--cmake-toolchain-for-android",
        help="Path to the cmake compiler toolchain",
        default="",
    )
    parser.add_argument(
        "--argparse-path",
        help="Path to argparse repo",
        default=f"{DEPENDENCY_DIR / 'argparse'}",
    )
    parser.add_argument(
        "--json-path",
        help="Path to nlohmann_json repo",
        default=f"{DEPENDENCY_DIR / 'json'}",
    )
    parser.add_argument(
        "--gtest-path",
        help="Path to gtest repo",
        default=f"{DEPENDENCY_DIR / 'googletest'}",
    )
    parser.add_argument(
        "--flatbuffers-path",
        help="Path to flatbuffers repo",
        default=f"{DEPENDENCY_DIR / 'flatbuffers'}",
    )
    parser.add_argument(
        "--enable-gcc-sanitizers",
        help="Enable GCC sanitizers. Default: %(default)s",
        action="store_true",
        default=False,
    )
    parser.add_argument(
        "--install",
        help="Install build artifacts into a provided location",
    )
    parser.add_argument(
        "--package",
        help="Create a package with build artifacts and store it in a provided location",
    )
    parser.add_argument(
        "--package-type",
        choices=["zip", "tgz", "pip"],
        help="Package type",
    )
    parser.add_argument(
        "--package-source",
        help="Create a source code package and store it in a provided location",
    )
    parser.add_argument(
        "--build-pylib",
        help="Build Python VGF Library. Default: %(default)s",
        action="store_true",
        default=False,
    )
    parser.add_argument(
        "--pybind11-path",
        help="Path to pybind11 repo",
        default=f"{DEPENDENCY_DIR / 'pybind11'}",
    )

    if argcomplete:
        argcomplete.autocomplete(parser)

    args = parser.parse_args()
    return args


def main():
    builder = Builder(parse_arguments())
    sys.exit(builder.run())


if __name__ == "__main__":
    main()
