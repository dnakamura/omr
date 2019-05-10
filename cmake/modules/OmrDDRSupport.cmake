###############################################################################
# Copyright (c) 2018, 2019 IBM Corp. and others
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
set(_OMR_DDR_SUPPORT 1)

include(OmrAssert)
include(OmrUtility)
include(ExternalProject)

set(OMR_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR})

function(make_ddr_set ddr_set)
	# if DDR is not enabled, just skip
	# Also skip if we are on windows since it is unsupported at the moment
	if((OMR_HOST_OS STREQUAL "win") OR (NOT OMR_DDR))
		return()
	endif()
	set(DDR_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/${ddr_set}")
	set(DDR_INFO_DIR "${CMAKE_BINARY_DIR}/ddr_info")
	set(DDR_SET_INFO_FILE "${DDR_INFO_ROOT}/sets/${ddr_set}")
	set(DDR_MACRO_INPUTS_FILE "${DDR_BIN_DIR}/macros.list")
	set(DDR_TOOLS_EXPORT "${omr_BINARY_DIR}/ddr/tools/DDRTools.cmake")
	set(DDR_CONFIG_STAMP "${DDR_BIN_DIR}/config.stamp")
	set(DDR_SET_NAME "${ddr_set}")

	add_custom_command(
		OUTPUT  "${DDR_CONFIG_STAMP}"
		COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
		COMMAND "${CMAKE_COMMAND}" -E touch "${DDR_CONFIG_STAMP}"
		WORKING_DIRECTORY "${DDR_BIN_DIR}"
	)
	add_custom_target(${ddr_set}
		DEPENDS "${DDR_CONFIG_STAMP}"
		COMMAND ${CMAKE_COMMAND} --build "${DDR_BIN_DIR}"
	)

	omr_join("\n" info_file_template
		"NAME"
		"${ddr_set}"
		"BINARY_DIR"
		"$<TARGET_PROPERTY:${ddr_set},BINARY_DIR>"
		"TARGETS"
		"$<JOIN:$<TARGET_PROPERTY:${ddr_set},DDR_TARGETS>,\n>"
		"SUBSETS"
		"$<TARGET_PROPERTY:${ddr_set},>"
		"$<JOIN:$<TARGET_PROPERTY:${ddr_set},DDR_SUBSETS>,\n>"

	)

	file(GENERATE OUTPUT "${DDR_INFO_DIR}/sets/${ddr_set}" CONTENT "${info_file_template}")

	# Note: DDR sets have themselves as targets to process
	# This is so that you can process misc headers which dont logicly belong to any other target
	target_enable_ddr_new(${ddr_set})
	set_property(TARGET "${ddr_set}" APPEND PROPERTY DDR_TARGETS "${ddr_set}")

	file(READ ${OMR_MODULES_DIR}/ddr/DDRSetStub.cmake.in cmakelist_template)
	string(CONFIGURE "${cmakelist_template}" cmakelist_template @ONLY)
	file(GENERATE OUTPUT ${DDR_BIN_DIR}/CMakeLists.txt CONTENT "${cmakelist_template}")
endfunction(make_ddr_set)

function(ddr_set_add_targets ddr_set)
	if((OMR_HOST_OS STREQUAL "win") OR (NOT OMR_DDR))
		return()
	endif()

	omr_assert(FATAL_ERROR TEST TARGET "${ddr_set}" MESSAGE "ddrset_add_targets called on non-existant ddr-set ${ddr_set}")

	foreach(tgt IN LISTS ARGN)
		omr_assert(FATAL_ERROR TEST TARGET ${tgt} MESSAGE "ddrset_add_targets called on non-existant target ${tgt}")
		# Check if the target we are adding is iteslf a DDR set
		get_target_property(tgt_is_ddr_set ${tgt} DDR_SET)
		if(tgt_is_ddr_set)
			set_property(TARGET ${ddr_set} APPEND PROPERTY DDR_SUBSETS ${tgt})
		else()
			set_property(TARGET ${ddr_set} APPEND PROPERTY DDR_TARGETS ${tgt})
		endif()
		add_dependencies(${ddr_set} ${tgt})
	endforeach()
endfunction(ddr_set_add_targets)

function(target_enable_ddr_new tgt)
	if((OMR_HOST_OS STREQUAL "win") OR (NOT OMR_DDR))
		return()
	endif()

	set(opt_EARLY_SOURCE_EVAL )
	set(opt_NO_SOURCE_EVAL )
	set(opt_UNPARSED_ARGUMENTS )
	cmake_parse_arguments(opt "EARLY_SOURCE_EVAL;NO_SOURCE_EVAL" "" "" ${ARGN})
	omr_assert(FATAL_ERROR TEST NOT opt_UNPARSED_ARGUMENTS MESSAGE "target_enable_ddr: unrecognized options ${opt_UNPARSED_ARGUMENTS}")

	omr_assert(FATAL_ERROR TEST TARGET ${tgt} MESSAGE "target_enable_ddr called on non-existant target ${tgt}")

	get_target_property(target_type "${tgt}" TYPE)
	if(target_type MATCHES "INTERFACE_LIBRARY")
		message(FATAL_ERROR "Cannot call enable_ddr on interface libraries")
	endif()

	if(opt_EARLY_SOURCE_EVAL)
		get_target_property(sources "${tgt}" "SOURCES")
		string(GENEX_STRIP "${sources}" cleaned_sources)
		set_target_properties("${tgt}" PROPERTIES DDR_EVAL_SOURCES "${cleaned_sources}")
		set(source_list  "$<JOIN:$<TARGET_PROPERTY:${tgt},DDR_EVAL_SOURCES>,\n>")
	elseif(opt_NO_SOURCE_EVAL)
		set(source_list "")
	else()
		set(source_list  "$<JOIN:$<TARGET_PROPERTY:${tgt},SOURCES>,\n>")
	endif()

	omr_join("\n" MAGIC_TEMPLATE
		"SOURCE_DIR"
		"$<TARGET_PROPERTY:${tgt},SOURCE_DIR>"
		"INCLUDE_PATH"
		"$<JOIN:$<TARGET_PROPERTY:${tgt},INCLUDE_DIRECTORIES>,\n>"
		"DEFINES"
		"$<JOIN:$<TARGET_PROPERTY:${tgt},COMPILE_DEFINITIONS>,\n>"
		"SOURCES"
		"${source_list}"
		"HEADERS"
		"$<JOIN:$<TARGET_PROPERTY:${tgt},DDR_HEADERS>,\n>"
		"PREINCLUDES"
		"$<JOIN:$<TARGET_PROPERTY:${tgt},DDR_PREINCLUDES>,\n>"
	)
	if(target_type MATCHES "EXECUTABLE|SHARED_LIBRARY")
		set(MAGIC_TEMPLATE "OUTPUT_FILE\n$<TARGET_FILE:${tgt}>\n${MAGIC_TEMPLATE}")
	endif()

	file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/ddr_info/targets/${tgt}.txt" CONTENT "${MAGIC_TEMPLATE}\n")
endfunction(target_enable_ddr_new)

function(target_enable_ddr tgt ddr_set)
	target_enable_ddr_new(${tgt} ${ARGN})
	ddr_set_add_targets(${ddr_set} ${tgt})
endfunction(target_enable_ddr)
