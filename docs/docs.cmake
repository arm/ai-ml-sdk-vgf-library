#
# SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

include(cmake/doxygen.cmake)
include(cmake/sphinx.cmake)

if(NOT DOXYGEN_FOUND OR NOT SPHINX_FOUND)
    return()
endif()

if(CMAKE_CROSSCOMPILING)
    message(WARNING "Cannot build the documentation when cross-compiling. Skipping.")
    return()
endif()

file(MAKE_DIRECTORY ${SPHINX_GEN_DIR})

# copy MD files for inclusion into the published docs
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CONTRIBUTING.md ${SPHINX_GEN_DIR}/CONTRIBUTING.md COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/README.md ${SPHINX_GEN_DIR}/README.md COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/SECURITY.md ${SPHINX_GEN_DIR}/SECURITY.md COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/LICENSES/Apache-2.0.txt ${SPHINX_GEN_DIR}/LICENSES/Apache-2.0.txt COPYONLY)

# Generate a text file with the VGF_DUMP tool help text
set(VGF_DUMP_ARG_HELP_TXT ${SPHINX_GEN_DIR}/vgf_dump_help.txt)
add_custom_command(
  OUTPUT "${VGF_DUMP_ARG_HELP_TXT}"
  COMMAND ${CMAKE_COMMAND}
          -Dcmd=$<IF:$<PLATFORM_ID:Windows>,.\\,./>$<TARGET_FILE_NAME:${VGF_NAMESPACE}::vgf_dump>
          -Dargs=--help
          -Dwd=$<TARGET_FILE_DIR:${VGF_NAMESPACE}::vgf_dump>
          -Dout=${VGF_DUMP_ARG_HELP_TXT}
          -P ${CMAKE_CURRENT_LIST_DIR}/redirect-output.cmake
  COMMAND_EXPAND_LISTS
  DEPENDS ${VGF_NAMESPACE}::vgf_dump
  BYPRODUCTS ${VGF_DUMP_ARG_HELP_TXT}
  VERBATIM
  COMMENT "Generating vgf_dump --help text"
)

set(DOC_SRC_FILES_FULL_PATHS
        ${SPHINX_GEN_DIR}/CONTRIBUTING.md
        ${SPHINX_GEN_DIR}/README.md
        ${SPHINX_GEN_DIR}/SECURITY.md
        ${VGF_DUMP_ARG_HELP_TXT})

# set source inputs list
file(GLOB_RECURSE  DOC_SRC_FILES CONFIGURE_DEPENDS RELATIVE ${SPHINX_SRC_DIR_IN} ${SPHINX_SRC_DIR_IN}/*)
foreach(SRC_IN IN LISTS DOC_SRC_FILES)
    set(DOC_SOURCE_FILE_IN "${SPHINX_SRC_DIR_IN}/${SRC_IN}")
    set(DOC_SOURCE_FILE "${SPHINX_SRC_DIR}/${SRC_IN}")
    configure_file(${DOC_SOURCE_FILE_IN} ${DOC_SOURCE_FILE} COPYONLY)
    list(APPEND DOC_SRC_FILES_FULL_PATHS ${DOC_SOURCE_FILE})
endforeach()

add_custom_command(
    OUTPUT ${SPHINX_INDEX_HTML}
    DEPENDS ${DOC_SRC_FILES_FULL_PATHS}
    COMMAND ${SPHINX_EXECUTABLE} -b html -W -Dbreathe_projects.MLSDK=${DOXYGEN_XML_GEN} ${SPHINX_SRC_DIR} ${SPHINX_BLD_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Sphinx"
    VERBATIM
)

# Prepare source files for API docs
file(GLOB_RECURSE  DOC_CODE_FILES_INCLUDE CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include/*)
file(GLOB_RECURSE  DOC_CODE_FILES_INCLUDE_C CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include-c/*)
file(GLOB_RECURSE  DOC_CODE_FILES_TEST CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/test/*)
set(DOC_CODE_FILES ${DOC_CODE_FILES_INCLUDE} ${DOC_CODE_FILES_INCLUDE_C} ${DOC_CODE_FILES_TEST})

set(DOC_CODE_FILES_FULL_PATHS "")
foreach(SRC_IN IN LISTS DOC_CODE_FILES)
    set(DOC_SOURCE_FILE_IN "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_IN}")
    set(DOC_SOURCE_FILE "${SPHINX_SOURCES_DIR}/${SRC_IN}")
    configure_file(${DOC_SOURCE_FILE_IN} ${DOC_SOURCE_FILE} COPYONLY)
    list(APPEND DOC_CODE_FILES_FULL_PATHS ${DOC_SOURCE_FILE})
endforeach()

# main target to build the docs
add_custom_target(vgf_doc ALL DEPENDS vgf_doxy_doc vgf_sphx_doc SOURCES "${SPHINX_SRC_DIR}/index.rst")
