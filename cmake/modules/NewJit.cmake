if(NEWJIT_INCLUDE_GUARD)
    return()
endif()
set(NEWJIT_INCLUDE_GUARD ON)


#unwrap a gnereator expression
#note this is very limited and only works as well as we need it to
# note we define out 
function(omr_unwrap_genex output genex)
    string(STRIP temp "${genex}")

    #remove any genexes of the form 
    set(temp "${genex}")


endfunction(omr_unwrap_genex)

# Handle all the generated sources
# process a number of sources and output modified list into OUTPUT_VAR
# usage: omr_compiler_process_generated(OUTPUT_VAR sources....)
function(omr_compiler_process_generated result_var)
    
    foreach(src_file IN LISTS ARGN)
        get_filename_component(extension ${in_f} EXT)
        if(extension STREQUAL ".asm")
            #TODO handle masm2gas
            message(FATAL_ERROR "masm2gas not implemented yet: ${src_file} " )
        endif()
    endforeach()
endfunction(omr_compiler_process_generated)


# Setup the current scope for compiling the Testarossa compiler technology. Used in
# conjunction with make_compiler_target -- Only can infect add_directory scope.
macro(set_tr_compile_options)
	omr_append_flags(CMAKE_CXX_FLAGS ${TR_COMPILE_OPTIONS} ${TR_CXX_COMPILE_OPTIONS})
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} PARENT_SCOPE)
	omr_append_flags(CMAKE_C_FLAGS ${TR_COMPILE_OPTIONS} ${TR_C_COMPILE_OPTIONS})
	set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} PARENT_SCOPE)
	# message("[set_tr_compile_options] Set CMAKE_CXX_FLAGS to ${CMAKE_CXX_FLAGS}")
	# message("[set_tr_compile_options] Set CMAKE_C_FLAGS to ${CMAKE_C_FLAGS}")
endmacro(set_tr_compile_options)


# Create an OMR Compiler component
#
# call like this:
#  create_omr_compiler_library(NAME <compilername>
#                              OBJECTS  <list of objects to add to the glue>
#                              FILTER   <list of default objects to remove from the compiler library.>
#                              INCLUDES <Additional includes for building the library>
#                              SHARED   <True if you want a shared object, false if you want a static archive>
#
# FILTER exists to allow compiler subprojects to opt-out of functionality
#        that they would prefer to replace.
#
function(create_omr_compiler_library)


	cmake_parse_arguments(COMPILER
		"SHARED" # Optional Arguments
		"NAME" # One value arguments
		"OBJECTS;DEFINES;FILTER;INCLUDES" # Multi value args
		${ARGV}
		)

     # Currently not doing cross, so assume HOST == TARGET
    set(TR_TARGET_ARCH    ${TR_HOST_ARCH})
    set(TR_TARGET_SUBARCH ${TR_HOST_SUBARCH})
    set(TR_TARGET_BITS    ${TR_HOST_BITS})
	if(COMPILER_SHARED)
		message("Creating shared library for ${COMPILER_NAME}")
		set(LIB_TYPE SHARED)
	else()
		message("Creating static library for ${COMPILER_NAME}")
		set(LIB_TYPE STATIC)
	endif()



	# Generate a build name file.
	set(BUILD_NAME_FILE "${CMAKE_BINARY_DIR}/${COMPILER_NAME}Name.cpp")
	add_custom_command(OUTPUT ${BUILD_NAME_FILE}
		COMMAND perl ${omr_SOURCE_DIR}/tools/compiler/scripts/generateVersion.pl ${COMPILER_NAME} > ${BUILD_NAME_FILE}
		VERBATIM
		COMMENT "Generate ${BUILD_NAME_FILE}"
	)

	#omr_inject_object_modification_targets(COMPILER_OBJECTS ${COMPILER_NAME} ${COMPILER_OBJECTS})

    #ewwww ewwww
    set_tr_compile_options()

	add_library(${COMPILER_NAME} ${LIB_TYPE}
		${BUILD_NAME_FILE}
		${COMPILER_OBJECTS}
    )
    
    #also pretty gross
    target_compile_definitions(${COMPILER_NAME}
        PUBLIC
        ${TR_COMPILE_DEFINITIONS}
    )


    target_link_libraries(${COMPILER_NAME} PRIVATE omr_compiler_base)
    #propogate the header stuffs
    # TODO force defining the compilers headers seems heavy handed
    target_include_directories(${COMPILER_NAME}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/${TR_TARGET_ARCH}/${TR_TARGET_SUBARCH}
            ${CMAKE_CURRENT_SOURCE_DIR}/${TR_TARGET_ARCH}
            ${CMAKE_CURRENT_SOURCE_DIR}
        INTERFACE
            $<TARGET_PROPERTY:omr_compiler_base,INTERFACE_INCLUDE_DIRECTORIES>
    )

	message(STATUS "arch = ${TR_TARGET_ARCH}")
	
	#omr_inject_object_modification_targets(CORE_COMPILER_OBJECTS ${COMPILER_NAME} ${CORE_COMPILER_OBJECTS})

	# Append to the compiler sources list
	# target_sources(${COMPILER_NAME} PRIVATE ${CORE_COMPILER_OBJECTS})

endfunction(create_omr_compiler_library)
