#
# SPDX-FileCopyrightText: Copyright 2023-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

set(SPHINX_BLD_DIR "${VGF_DOCS_DIR}/out")
set(SPHINX_INDEX_HTML "${SPHINX_BLD_DIR}/index.html")
set(SPHINX_STAMP "${VGF_DOCS_DIR}/sphinx.stamp")
set(DOXYGEN_XML_GEN "${DOXYGEN_OUTPUT_DIR}/xml")

if(VGF_DOCS_STANDALONE)
    set(VGF_SPHINX_COMMAND "${SPHINX_EXECUTABLE}")
else()
    set(VGF_SPHINX_COMMAND "${CMAKE_COMMAND}" -E env
        "PYTHONPATH=$<TARGET_FILE_DIR:vgfpy>" "${SPHINX_EXECUTABLE}")
endif()

add_custom_command(
    OUTPUT "${SPHINX_STAMP}"
    BYPRODUCTS "${SPHINX_INDEX_HTML}"
    DEPENDS ${DOC_STAGED_FILES} "${DOC_STAGED_MANIFEST}"
        "${DOXYGEN_STAMP}" "${DOXYGEN_INDEX_XML}"
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${SPHINX_BLD_DIR}"
    COMMAND ${VGF_SPHINX_COMMAND} -b html -W
        "-Dbreathe_projects.MLSDK=${DOXYGEN_XML_GEN}"
        "${SPHINX_SRC_DIR}" "${SPHINX_BLD_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E touch "${SPHINX_STAMP}"
    WORKING_DIRECTORY "${VGF_DOCS_BINARY_DIR}"
    COMMENT "Generating API documentation with Sphinx"
    VERBATIM)

if(VGF_DOCS_STANDALONE)
    add_custom_target(vgf_sphx_doc
        DEPENDS "${SPHINX_STAMP}" "${SPHINX_INDEX_HTML}" vgf_doxy_doc)
else()
    add_custom_target(vgf_sphx_doc
        DEPENDS "${SPHINX_STAMP}" "${SPHINX_INDEX_HTML}" vgf_doxy_doc vgfpy)
endif()
