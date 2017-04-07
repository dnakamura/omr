add_executable(tracegen
	main.cpp
)

target_link_libraries(tracegen
	PRIVATE
		trace # static
)

# TODO: Move libtrace to it's own home
# NOTE: trace is static so we don't futz with RPATHs to run the tools.
add_library(trace STATIC
	ArgParser.cpp
	CFileWriter.cpp
	DATFileWriter.cpp
	FileReader.cpp
	FileUtils.cpp
	Port.cpp
	StringUtils.cpp
	TDFParser.cpp
	TraceGen.cpp
	TraceHeaderWriter.cpp
)

target_include_directories(trace
	INTERFACE
		./
)

if(OMR_HOST_OS STREQUAL "zos")
	target_link_libraries(trace j9a2e)
endif()
