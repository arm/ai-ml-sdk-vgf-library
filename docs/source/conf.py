#
# SPDX-FileCopyrightText: Copyright 2022-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
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
    "sphinx.ext.autosectionlabel",
    "myst_parser",
    "sphinxcontrib.plantuml",
]

# Match the SDK documentation's scalable HTML diagrams.
plantuml_output_format = "svg_img"

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
