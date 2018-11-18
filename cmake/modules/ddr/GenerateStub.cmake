
message(STATUS "DBG1 in='${input_file}'")
execute_process(COMMAND grep -lE "@ddr_(namespace|options):" ${input_file} RESULT_VARIABLE rc)
message(STATUS "DBG2")
if(rc)
    #input didnt have any ddr directives, so just dump an empty file
    file(WRITE ${output_file} "")
else()
    file(REMOVE ${output_file})

    execute_process(COMMAND awk -f ${AWK_SCRIPT} ${input_file} OUTPUT_VARIABLE awk_result RESULT_VARIABLE rc)
    if(NOT ${rc})
        file(WRITE ${output_file} "#include \"${input_file}\"\n")
        file(APPEND ${output_file} "${awk_result}")
    endif()
    return(${rc})
endif()
