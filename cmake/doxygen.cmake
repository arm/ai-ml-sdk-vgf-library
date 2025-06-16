#
# SPDX-FileCopyrightText: Copyright 2023-2024 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#

find_package(Doxygen)
if(NOT DOXYGEN_FOUND)
    message("Doxygen is not available. Cannot generate documentation.")
    unset(DOXYGEN_EXECUTABLE CACHE)
    return()
endif()

set(DOXYFILE_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile)
set(DOXYFILE_GEN ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
set(DOXYGEN_INPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/docs/sources)
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/docs/doxygen)
set(DOXYGEN_INDEX_HTML ${DOXYGEN_OUTPUT_DIR}/html/index.html)
set(DOXYGEN_INDEX_XML ${DOXYGEN_OUTPUT_DIR}/xml/index.xml)

configure_file(${DOXYFILE_CONFIG} ${DOXYFILE_GEN} @ONLY)

file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})

set(DOXYGEN_OUTPUT ${DOXYGEN_INDEX_HTML} ${DOXYGEN_INDEX_XML})
add_custom_command(
    OUTPUT ${DOXYGEN_OUTPUT}
    MAIN_DEPENDENCY ${DOXYFILE_CONFIG} ${DOXYFILE_GEN}
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_GEN}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)

add_custom_target(vgf_doxy_doc DEPENDS ${DOXYGEN_OUTPUT})
