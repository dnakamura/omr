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
    file(GENERATE OUTPUT ${DDR_DEBUG_INPUTS_FILE} CONTENT "$<JOIN:$<TARGET_PROPERTY:${DDR_TARGET_NAME},DDR_DEBUG_INPUTS>,\n>\n")

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

function(target_enable_ddr tgt)
    omr_assert(FATAL_ERROR TEST TARGET ${tgt} MESSAGE "target_enable_ddr called on non-existant target ${tgt}")

    get_target_property(target_bin_dir ${tgt} BINARY_DIR)
    get_target_property(DDR_SOURCE_DIR ${tgt} SOURCE_DIR)

    set(DDR_BIN_DIR "${target_bin_dir}/${tgt}_ddr")
    set(DDR_SOURCES_LIST ${DDR_BIN_DIR}/input.list)
    set(DDR_SUPPORT_DIR ${OMR_MODULES_DIR}/ddr)
    set(DDR_PROJECT_NAME ${tgt}_ddr)
    

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
    
    set_target_properties(${tgt} PROPERTIES DDR_MACRO_LIST ${DDR_BIN_DIR}/macro_list)
endfunction(target_enable_ddr)



