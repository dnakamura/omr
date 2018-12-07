if(_OMR_DDR_SUPPORT)
    return()
endif()
include(OmrAssert)
include(ExternalProject)
set(_OMR_DDR_SUPPORT 1)

set(OMR_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR})


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



