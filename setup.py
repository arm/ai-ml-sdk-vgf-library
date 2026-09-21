#
# SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
import pathlib
import sys

from setuptools import Extension
from setuptools import setup
from setuptools.command.build_ext import build_ext
from setuptools.command.build_py import build_py

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


setup(
    cmdclass={"build_ext": BuildExt, "build_py": BuildPy},
    ext_modules=[CMakeExtension("vgfpy")],
)
