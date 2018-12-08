###############################################################################
# Copyright (c) 2018, 2018 IBM Corp. and others
#
# This program and the accompanying materials are made available under
# the terms of the Eclipse Public License 2.0 which accompanies this
# distribution and is available at http://eclipse.org/legal/epl-2.0
# or the Apache License, Version 2.0 which accompanies this distribution
# and is available at https://www.apache.org/licenses/LICENSE-2.0.
#
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the
# Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
# version 2 with the GNU Classpath Exception [1] and GNU General Public
# License, version 2 with the OpenJDK Assembly Exception [2].
#
# [1] https://www.gnu.org/software/classpath/license.html
# [2] http://openjdk.java.net/legal/assembly-exception.html
#
# SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
#############################################################################

if(_OMR_DDR_SUPPORT)
    return()
endif()
include(OmrAssert)
include(ExternalProject)
set(_OMR_DDR_SUPPORT 1)

set(OMR_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR})

function(make_ddr_set ddr_set)
    set(DDR_TARGET_NAME "${ddr_set}_ddr")
    set(DDR_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/${DDR_TARGET_NAME})
    set(DDR_MACRO_INPUTS_FILE "${DDR_BIN_DIR}/macros.list")
    set(DDR_DEBUG_INPUTS_FILE "${DDR_BIN_DIR}/debug.list")


    file(MAKE_DIRECTORY "${DDR_BIN_DIR}")


    file(GENERATE OUTPUT ${DDR_MACRO_INPUTS_FILE} CONTENT "$<JOIN:$<TARGET_PROPERTY:${DDR_TARGET_NAME},DDR_MACRO_INPUTS>,\n>\n")
    #TODO $<GENEX_EVAL:...> actually requires a fairly recent version of cmake. Need to figure out an alternate method
    file(GENERATE OUTPUT ${DDR_DEBUG_INPUTS_FILE} CONTENT "$<JOIN:$<GENEX_EVAL:$<TARGET_PROPERTY:${DDR_TARGET_NAME},DDR_DEBUG_INPUTS>>,\n>\n")

    file(READ ${OMR_MODULES_DIR}/ddr/DDRSetStub.cmake.in cmakelist_template)
    string(CONFIGURE "${cmakelist_template}" cmakelist_template @ONLY)
    file(GENERATE OUTPUT ${DDR_BIN_DIR}/CMakeLists.txt CONTENT "${cmakelist_template}")

    add_custom_command(
        OUTPUT ${DDR_BIN_DIR}/config.stamp
        COMMAND ${CMAKE_COMMAND} .
        COMMAND ${CMAKE_COMMAND} -E touch config.stamp
        WORKING_DIRECTORY ${DDR_BIN_DIR}
    )

    add_custom_target(${DDR_TARGET_NAME}
        DEPENDS ${DDR_BIN_DIR}/config.stamp
        COMMAND ${CMAKE_COMMAND} --build ${DDR_BIN_DIR}
    )
endfunction(make_ddr_set)

function(target_enable_ddr tgt ddr_set)
    set(DDR_SET_TARGET "${ddr_set}_ddr")
    omr_assert(FATAL_ERROR TEST TARGET ${tgt} MESSAGE "target_enable_ddr called on non-existant target ${tgt}")
    omr_assert(FATAL_ERROR TEST TARGET "${DDR_SET_TARGET}" MESSAGE "target_enable_ddr called on non-existant ddr_set ${ddr_set}")

    get_target_property(target_bin_dir ${tgt} BINARY_DIR)
    get_target_property(DDR_SOURCE_DIR ${tgt} SOURCE_DIR)
    get_target_property(target_type ${tgt} TYPE)

    if(target_type MATCHES "INTERFACE_LIBRARY|OBJECT_LIBRARY")
        message(FATAL_ERROR "Cannot call enable_ddr on interface or object libraries")
    endif()

    set(DDR_BIN_DIR "${target_bin_dir}/${tgt}_ddr")
    set(DDR_SOURCES_LIST ${DDR_BIN_DIR}/sources_list)
    set(DDR_SUPPORT_DIR ${OMR_MODULES_DIR}/ddr)
    set(DDR_PROJECT_NAME ${tgt}_ddr)
    set(DDR_MACRO_LIST "${DDR_BIN_DIR}/macro_list")
    

    set(DDR_PREPROCESSOR_COMMAND "gcc -xc -E")
    file(GENERATE OUTPUT ${DDR_SOURCES_LIST} CONTENT "$<JOIN:$<TARGET_PROPERTY:${tgt},DDR_HEADERS>,\n>\n$<JOIN:$<TARGET_PROPERTY:${tgt},SOURCES>,\n>\n")

    configure_file(${OMR_MODULES_DIR}/DDRStub.cmake ${DDR_BIN_DIR}/CMakeLists.txt @ONLY)

    add_custom_command(
        OUTPUT ${DDR_BIN_DIR}/config.stamp
        COMMAND ${CMAKE_COMMAND} .
        COMMAND ${CMAKE_COMMAND} -E touch config.stamp
        WORKING_DIRECTORY ${DDR_BIN_DIR}
    )

    add_custom_target(${tgt}_ddrgen
        DEPENDS ${DDR_BIN_DIR}/config.stamp
        COMMAND ${CMAKE_COMMAND} --build ${DDR_BIN_DIR}
    )
    
    set_target_properties(${tgt} PROPERTIES DDR_MACRO_LIST ${DDR_MACRO_LIST})

    add_dependencies(${DDR_SET_TARGET} "${tgt}_ddrgen")
    set_property(TARGET ${DDR_SET_TARGET} APPEND PROPERTY DDR_MACRO_INPUTS ${DDR_MACRO_LIST})
    if(target_type MATCHES "SHARED_LIBRARY|EXECUTABLE")
        #TODO this doesnt handle pdbs
        set_property(TARGET ${DDR_SET_TARGET} APPEND PROPERTY DDR_DEBUG_INPUTS "$<TARGET_FILE:${tgt}>")
    endif()

endfunction(target_enable_ddr)
