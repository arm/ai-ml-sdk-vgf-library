#
# SPDX-FileCopyrightText: Copyright 2023-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

get_filename_component(VGF_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(VGF_DOCS_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(VGF_DOCS_DIR "${VGF_DOCS_BINARY_DIR}/docs")
list(APPEND CMAKE_MODULE_PATH "${VGF_REPO_ROOT}/cmake")

find_package(Doxygen 1.9.1 REQUIRED)
find_package(Sphinx REQUIRED)

set(SPHINX_SRC_DIR_IN "${VGF_REPO_ROOT}/docs/source")
set(SPHINX_SRC_DIR "${VGF_DOCS_DIR}/in")
set(SPHINX_GEN_DIR "${VGF_DOCS_DIR}/generated")
set(SPHINX_SOURCES_DIR "${VGF_DOCS_DIR}/sources")
set(DOC_STAGED_FILES)

foreach(SRC_IN README.md CONTRIBUTING.md SECURITY.md LICENSES/Apache-2.0.txt)
    set(DOC_SOURCE_FILE "${SPHINX_GEN_DIR}/${SRC_IN}")
    configure_file("${VGF_REPO_ROOT}/${SRC_IN}" "${DOC_SOURCE_FILE}" COPYONLY)
    list(APPEND DOC_STAGED_FILES "${DOC_SOURCE_FILE}")
endforeach()

foreach(HELP_NAME vgf_dump vgf_updater)
    set(DOC_SOURCE_FILE "${SPHINX_GEN_DIR}/${HELP_NAME}_help.txt")
    configure_file("${VGF_REPO_ROOT}/docs/help/${HELP_NAME}_help.rst"
        "${DOC_SOURCE_FILE}" COPYONLY)
    list(APPEND DOC_STAGED_FILES "${DOC_SOURCE_FILE}")
endforeach()

file(GLOB_RECURSE DOC_SRC_FILES CONFIGURE_DEPENDS LIST_DIRECTORIES false
    RELATIVE "${SPHINX_SRC_DIR_IN}" "${SPHINX_SRC_DIR_IN}/*")
foreach(SRC_IN IN LISTS DOC_SRC_FILES)
    set(DOC_SOURCE_FILE "${SPHINX_SRC_DIR}/${SRC_IN}")
    configure_file("${SPHINX_SRC_DIR_IN}/${SRC_IN}" "${DOC_SOURCE_FILE}" COPYONLY)
    list(APPEND DOC_STAGED_FILES "${DOC_SOURCE_FILE}")
endforeach()

# Stage public headers for the combined SDK reference and the four test
# snippets rendered by literalinclude in the VGF API pages.
file(GLOB DOC_CODE_FILES CONFIGURE_DEPENDS LIST_DIRECTORIES false
    RELATIVE "${VGF_REPO_ROOT}"
    "${VGF_REPO_ROOT}/include/vgf/*.h" "${VGF_REPO_ROOT}/include/vgf/*.hpp"
    "${VGF_REPO_ROOT}/include-c/vgf/*.h" "${VGF_REPO_ROOT}/include-c/vgf/*.hpp")
list(APPEND DOC_CODE_FILES
    test/header_tests.cpp test/model_sequence_tests.cpp
    test/model_resource_tests.cpp test/module_table_tests.cpp)
foreach(SRC_IN IN LISTS DOC_CODE_FILES)
    set(DOC_SOURCE_FILE "${SPHINX_SOURCES_DIR}/${SRC_IN}")
    configure_file("${VGF_REPO_ROOT}/${SRC_IN}" "${DOC_SOURCE_FILE}" COPYONLY)
    list(APPEND DOC_STAGED_FILES "${DOC_SOURCE_FILE}")
endforeach()

# The Doxyfile reads only top-level files in the two public API directories.
file(GLOB DOC_CODE_FILES_DOXYGEN CONFIGURE_DEPENDS LIST_DIRECTORIES false
    "${VGF_REPO_ROOT}/include/vgf/*" "${VGF_REPO_ROOT}/include-c/vgf/*")
set(DOC_DOXYGEN_FILES)
foreach(DOC_SOURCE_FILE IN LISTS DOC_CODE_FILES_DOXYGEN)
    if(DOC_SOURCE_FILE MATCHES "\\.(h|hpp)$")
        list(APPEND DOC_DOXYGEN_FILES "${DOC_SOURCE_FILE}")
    endif()
endforeach()

set(DOC_STAGED_MANIFEST "${VGF_DOCS_DIR}/staged-files.txt")
if(EXISTS "${DOC_STAGED_MANIFEST}")
    file(STRINGS "${DOC_STAGED_MANIFEST}" DOC_PREVIOUS_FILES)
    foreach(DOC_PREVIOUS_FILE IN LISTS DOC_PREVIOUS_FILES)
        cmake_path(IS_PREFIX VGF_DOCS_DIR "${DOC_PREVIOUS_FILE}" NORMALIZE DOC_IS_STAGED)
        if(DOC_IS_STAGED AND NOT DOC_PREVIOUS_FILE IN_LIST DOC_STAGED_FILES)
            file(REMOVE "${DOC_PREVIOUS_FILE}")
        endif()
    endforeach()
endif()
list(JOIN DOC_STAGED_FILES "\n" DOC_STAGED_CONTENT)
file(CONFIGURE OUTPUT "${DOC_STAGED_MANIFEST}" CONTENT "${DOC_STAGED_CONTENT}\n" @ONLY)
set(DOC_DOXYGEN_MANIFEST "${VGF_DOCS_DIR}/doxygen-inputs.txt")
list(JOIN DOC_DOXYGEN_FILES "\n" DOC_DOXYGEN_CONTENT)
file(CONFIGURE OUTPUT "${DOC_DOXYGEN_MANIFEST}" CONTENT "${DOC_DOXYGEN_CONTENT}\n" @ONLY)

include("${VGF_REPO_ROOT}/cmake/doxygen.cmake")
include("${VGF_REPO_ROOT}/cmake/sphinx.cmake")

add_custom_target(vgf_doc ALL
    DEPENDS vgf_doxy_doc vgf_sphx_doc
    SOURCES "${SPHINX_SRC_DIR}/index.rst")
