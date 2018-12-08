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

cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

set(DDR_PREPROCESSOR_COMMAND @DDR_PREPROCESSOR_COMMAND@)
set(DDR_SOURCES_LIST "@DDR_SOURCES_LIST@")
set(DDR_SOURCE_DIR "@DDR_SOURCE_DIR@")
set(DDR_PROCESS_SCRIPT "@DDR_PROCESS_SCRIPT@")
set(DDR_SUPPORT_DIR "@DDR_SUPPORT_DIR@")
#set(DDR_GEN_I_CMD "@DDR_GEN_I_CMD@")


project(@DDR_PROJECT_NAME@ LANGUAGES NONE)

file(STRINGS ${DDR_SOURCES_LIST} input_list)
set(processed_files "")
set(annt_files "")



foreach(input_file IN LISTS input_list)

    get_filename_component(abs_file ${input_file} ABSOLUTE BASE_DIR ${DDR_SOURCE_DIR})
    get_filename_component(file_name ${abs_file} NAME_WE)

    if(${file_name} IN_LIST processed_files)
        message(FATAL_ERROR "duplicate source")
    endif()
    list(APPEND processed_files ${file_name})
    set(stub_file ${CMAKE_CURRENT_BINARY_DIR}/${file_name}.stub.c)
    set(annt_file ${CMAKE_CURRENT_BINARY_DIR}/${file_name}.annt)



    add_custom_command(
        OUTPUT ${stub_file}
        DEPENDS
            ${abs_file}
            ${DDR_SUPPORT_DIR}/cmake_ddr.awk
            ${DDR_SUPPORT_DIR}/GenerateStub.cmake
        COMMAND ${CMAKE_COMMAND} -DAWK_SCRIPT=${DDR_SUPPORT_DIR}/cmake_ddr.awk -Dinput_file=${abs_file} -Doutput_file=${stub_file} -P ${DDR_SUPPORT_DIR}/GenerateStub.cmake
    )

    add_custom_command(
        OUTPUT ${annt_file}
        DEPENDS ${stub_file}
        COMMAND 
            @DDR_PREPROCESSOR_COMMAND@ ${stub_file} |
            awk "/^\$/{next} /^DDRFILE_BEGIN /,/^DDRFILE_END /{print \"@\" $0}"
             > ${annt_file}
        VERBATIM
        #COMMAND ${DDR_PREPROCESSOR_COMMAND} ${i_file} > ${annt_file}

    )


    list(APPEND annt_files ${annt_file})
endforeach()

add_custom_command(
    OUTPUT macro_list
    DEPENDS ${annt_files}
    COMMAND cat ${annt_files} > macro_list

)
add_custom_target(dummygen ALL
    DEPENDS
    macro_list
)
