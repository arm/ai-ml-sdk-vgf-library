#
# SPDX-FileCopyrightText: Copyright 2022-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
import os
import re
import sys
from importlib import import_module

from docutils import nodes
from docutils.parsers.rst import Directive
from docutils.statemachine import StringList

sys.path.insert(0, os.path.abspath("."))

# ML SDK VGF Library project config
VGF_project = "ML SDK VGF Library"
copyright = "2022-2026, Arm Limited and/or its affiliates <open-source-office@arm.com>"
author = "Arm Limited"
git_repo_tool_url = "https://gerrit.googlesource.com/git-repo"

# Set home project name
project = VGF_project

rst_epilog = """
.. |VGF_project| replace:: %s
.. |git_repo_tool_url| replace:: %s
""" % (
    VGF_project,
    git_repo_tool_url,
)

# Enabled extensions
extensions = [
    "breathe",
    "sphinx_rtd_theme",
    "sphinx.ext.autodoc",
    "sphinx.ext.autosectionlabel",
    "myst_parser",
]

autodoc_default_options = {
    "undoc-members": True,
}

# Disable converting double-dash to typographical en-dash
smartquotes_action = "qe"

# Disable superfluous warnings
suppress_warnings = [
    "autosectionlabel.*",
    "myst.xref_missing",
    "myst.header",
]

# Breathe Configuration
breathe_projects = {"VGF": "../generated/xml"}
breathe_default_project = "VGF-Lib"
breathe_domain_by_extension = {"h": "c"}

# Enable RTD theme
html_theme = "sphinx_rtd_theme"

tags.add("WITH_BASE_MD")


class PyEnumTable(Directive):
    """Generate a compact table of enum members from the Python module."""

    has_content = False

    def run(self):
        module = import_module("vgfpy")
        enums = sorted(
            (name, value)
            for name, value in vars(module).items()
            if isinstance(value, type) and hasattr(value, "__members__")
        )
        lines = [
            ".. list-table::",
            "   :header-rows: 1",
            "   :widths: 30 70",
            "",
            "   * - Enumeration",
            "     - Members",
        ]
        for name, enum in enums:
            members = ", ".join(f"``{member}``" for member in enum.__members__)
            lines.extend([f"   * - ``{name}``", f"     - {members}"])

        container = nodes.container()
        self.state.nested_parse(StringList(lines), self.content_offset, container)
        return container.children


def normalize_pybind11_docstrings(app, what, name, obj, options, lines):
    """Render pybind11's overload signatures as literals in autodoc output."""
    if what not in {"function", "method"}:
        return

    for index, line in enumerate(lines):
        if re.match(r"^\d+\. [A-Za-z_][A-Za-z0-9_]*\(", line):
            lines[index] = f"``{line}``"


def hide_pybind11_wrapper_signature(
    app, what, name, obj, options, signature, return_annotation
):
    """Prefer pybind11's detailed overload signatures to its *args wrapper."""
    if signature == "(*args, **kwargs)":
        return "", return_annotation

    return signature, return_annotation


def setup(app):
    app.add_directive("py-enum-table", PyEnumTable)
    app.connect("autodoc-process-docstring", normalize_pybind11_docstrings)
    app.connect("autodoc-process-signature", hide_pybind11_wrapper_signature)
