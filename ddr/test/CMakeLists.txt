
include(OmrDDRSupport)

add_executable(ddrgentest
    test_subdir/testFileInSubDir.cpp
    sample1.cpp
    sample2.cpp
    sample3.cpp
    sample4.c
    test.cpp
)

target_enable_ddr(ddrgentest)

file(GLOB_RECURSE headerfiles "${CMAKE_CURRENT_SOURCE_DIR}/*.h[pp]?")
set_property(TARGET ddrgentest PROPERTY DDR_HEADERS ${headerfiles})
