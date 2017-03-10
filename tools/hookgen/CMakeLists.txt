
add_executable(hookgen
	HookGen.cpp
	main.cpp
)

target_link_libraries(hookgen pugixml)
if(OMR_HOST_OS STREQUAL "zos")
	target_link_libraries(hookgen j9a2e)
endif()
