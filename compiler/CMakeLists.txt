
set(TR_TARGET_ARCH    ${TR_HOST_ARCH})
set(TR_TARGET_SUBARCH ${TR_HOST_SUBARCH})
set(TR_TARGET_BITS    ${TR_HOST_BITS})

#TODO this is pretty hack-y
function(compiler_library name )
    #TODO we need to handle cases with generated source files
    target_sources(omr_compiler_base INTERFACE ${ARGN})
endfunction(compiler_library)

# interface library which contains flags/definitions you need for linking
add_library(omr_compiler_interface INTERFACE)
target_compile_definitions(omr_compiler_interface
    INTERFACE

)
target_compile_options(omr_compiler_interface
    INTERFACE
    #${CMAKE_CXX_FLAGS}
)



add_library(omr_compiler_base INTERFACE)
target_include_directories(omr_compiler_base
    INTERFACE
        ${omr_SOURCE_DIR}/compiler/${TR_TARGET_ARCH}/${TR_TARGET_SUBARCH}
		${omr_SOURCE_DIR}/compiler/${TR_TARGET_ARCH}
		${omr_SOURCE_DIR}/compiler
		${omr_SOURCE_DIR}
)

add_subdirectory(codegen)
add_subdirectory(compile)
add_subdirectory(control)
add_subdirectory(env)
add_subdirectory(il)
add_subdirectory(ilgen)
add_subdirectory(infra)
add_subdirectory(optimizer)
add_subdirectory(ras)
add_subdirectory(runtime)

add_subdirectory(${TR_TARGET_ARCH})
