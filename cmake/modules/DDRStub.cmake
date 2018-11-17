
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
    set(i_file ${CMAKE_CURRENT_BINARY_DIR}/${file_name}.stub.c)
    set(annt_file ${CMAKE_CURRENT_BINARY_DIR}/${file_name}.annt)



    add_custom_command(
        OUTPUT ${i_file}
        DEPENDS ${abs_file}
        COMMAND ${CMAKE_COMMAND} -DAWK_SCRIPT=${DDR_SUPPORT_DIR}/cmake_ddr.awk -Dinput_file=${abs_file} -Doutput_file=${i_file} -P ${DDR_SUPPORT_DIR}/AnnotateHeader.cmake
    )

    add_custom_command(
        OUTPUT ${annt_file}
        DEPENDS ${i_file}
        COMMAND 
            @DDR_PREPROCESSOR_COMMAND@ ${i_file} #|
            #awk "/^DDRFILE_BEGIN /,/^DDRFILE_END /{print \"@\" $0}"
             > ${annt_file}
        VERBATIM
        #COMMAND ${DDR_PREPROCESSOR_COMMAND} ${i_file} > ${annt_file}

    )


    list(APPEND annt_files ${annt_file})
endforeach()

add_custom_target(dummygen ALL
    DEPENDS
    ${annt_files}
)
