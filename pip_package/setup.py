#
# SPDX-FileCopyrightText: Copyright 2025-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
import importlib.machinery
import pathlib
import platform
import shutil

from setuptools import Extension
from setuptools import setup
from setuptools.command.build_ext import build_ext
from wheel.bdist_wheel import bdist_wheel


# vgfpy is built by CMake before this package is assembled, so setuptools
# needs a marker extension type for a prebuilt binary rather than sources.
class PrebuiltExtension(Extension):
    def __init__(self, name, path):
        super().__init__(name, sources=[])
        self.path = path


# setuptools calls build_extension for each item in ext_modules. For vgfpy,
# copy the prebuilt binary into the expected wheel location instead of compiling.
class BuildExt(build_ext):
    def build_extension(self, ext):
        if isinstance(ext, PrebuiltExtension):
            output_path = pathlib.Path(self.get_ext_fullpath(ext.name))
            output_path.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ext.path, output_path)
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
            platformName = "win_amd64"
        elif system == "Linux":
            if machine == "aarch64":
                platformName = "manylinux2014_aarch64"
            else:
                assert machine == "x86_64"
                platformName = "manylinux2014_x86_64"
        elif system == "Darwin":
            assert machine == "arm64"
            platformName = "macosx_11_0_arm64"
        python_tag, abi_tag, _ = super().get_tag()
        return (python_tag, abi_tag, platformName)


def get_prebuilt_extension():
    package_dir = pathlib.Path(__file__).parent
    for suffix in importlib.machinery.EXTENSION_SUFFIXES:
        extension_path = package_dir / f"vgfpy{suffix}"
        if extension_path.exists():
            return PrebuiltExtension("vgfpy", str(extension_path))
    return None


prebuilt_extension = get_prebuilt_extension()

setup(
    cmdclass={"bdist_wheel": BDistWheel, "build_ext": BuildExt},
    ext_modules=[prebuilt_extension] if prebuilt_extension else [],
)
