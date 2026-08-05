#
# SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
import pathlib
import platform
import sys

from setuptools import Extension
from setuptools import setup
from setuptools.command.build_ext import build_ext
from setuptools.command.build_py import build_py
from wheel.bdist_wheel import bdist_wheel

ROOT_DIR = pathlib.Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT_DIR / "pip_package"))

from build_native import build_native  # noqa: E402


class CMakeExtension(Extension):
    def __init__(self, name):
        super().__init__(name, sources=[])


class BuildPy(build_py):
    def run(self):
        self.run_command("build_ext")
        super().run()


class BuildExt(build_ext):
    def build_extension(self, ext):
        if isinstance(ext, CMakeExtension):
            output_path = pathlib.Path(self.get_ext_fullpath(ext.name)).resolve()
            install_dir = None
            if self.editable_mode:
                build_py = self.get_finalized_command("build_py")
                install_dir = pathlib.Path(build_py.get_package_dir("vgf_lib"))
                install_dir /= "binaries"
            build_native(
                output_path,
                install_dir,
            )
            return

        super().build_extension(ext)


class BDistWheel(bdist_wheel):
    def finalize_options(self):
        super().finalize_options()
        self.root_is_pure = False

    def get_tag(self):
        system = platform.system()
        machine = platform.machine()
        if system == "Windows":
            assert machine == "AMD64"
            platform_name = "win_amd64"
        elif system == "Linux":
            if machine == "aarch64":
                platform_name = "manylinux2014_aarch64"
            else:
                assert machine == "x86_64"
                platform_name = "manylinux2014_x86_64"
        elif system == "Darwin":
            assert machine == "arm64"
            platform_name = "macosx_11_0_arm64"
        else:
            raise RuntimeError(f"Unsupported platform: {system} {machine}")

        python_tag, abi_tag, _ = super().get_tag()
        return (python_tag, abi_tag, platform_name)


setup(
    cmdclass={"bdist_wheel": BDistWheel, "build_ext": BuildExt, "build_py": BuildPy},
    ext_modules=[CMakeExtension("vgfpy")],
)
