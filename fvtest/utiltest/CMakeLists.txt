add_executable(omrutiltest
	main.cpp
)

target_link_libraries(omrutiltest
	#omrGtestGlue
	omrGtest
	omrutil
	omrglue
)

target_include_directories(omrutiltest
	PRIVATE
	$<TARGET_PROPERTY:omrGtestGlue,INTERFACE_INCLUDE_DIRECTORIES>
)

if(OMR_HOST_OS STREQUAL "zos")
	target_link_libraries(omrutiltest j9a2e)
endif()

add_test(NAME utiltest COMMAND omrutiltest)
