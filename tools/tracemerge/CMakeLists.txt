add_executable(tracemerge
	DATMerge.cpp
	main.cpp
)

target_include_directories(tracemerge
	PRIVATE
		../tracegen/
)

target_link_libraries(tracemerge
	PRIVATE
		trace # static
)
