

set(JIT_PRODUCT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(JIT_OMR_DIRTY_DIR ${CMAKE_SOURCE_DIR}/compiler)

Find_package(Perl)

if (NOT PERL_FOUND )
	message(FATAL_ERROR "Perl not found")
else()
	message(STATUS PERL = ${PERL_EXECUTABLE})
endif()


#TODO this is horribly broken
set(HOST_ARCH x)
set(HOST_SUBARCH amd64)


set(TARGET_ARCH ${HOST_ARCH})
set(TARGET_SUBARCH ${HOST_SUBARCH})


include_directories(
	${TARGET_ARCH}/${TARGET_SUBARCH}
	$(TARGET_ARCH) 
	. 
	.. 
	${JIT_OMR_DIRTY_DIR}/${TARGET_ARCH}/${TARGET_SUBARCH}
	${JIT_OMR_DIRTY_DIR}/${TARGET_ARCH} 
	${JIT_OMR_DIRTY_DIR} 
	${CMAKE_SOURCE_DIR} 
)

set (JIT_PRODUCT_BACKEND_SOURCES )
set(JIT_PRODUCT_SOURCE_FILES
	${JIT_PRODUCT_DIR}/ilgen/BinaryOpIlInjector.cpp
	${JIT_PRODUCT_DIR}/ilgen/ChildlessUnaryOpIlInjector.cpp
	${JIT_PRODUCT_DIR}/ilgen/CmpBranchOpIlInjector.cpp
	${JIT_PRODUCT_DIR}/ilgen/OpIlInjector.cpp
	${JIT_PRODUCT_DIR}/ilgen/StoreOpIlInjector.cpp
	${JIT_PRODUCT_DIR}/ilgen/TernaryOpIlInjector.cpp
	${JIT_PRODUCT_DIR}/ilgen/UnaryOpIlInjector.cpp
	${JIT_PRODUCT_DIR}/tests/BarIlInjector.cpp
	${JIT_PRODUCT_DIR}/tests/BuilderTest.cpp
	${JIT_PRODUCT_DIR}/tests/CallIlInjector.cpp
	${JIT_PRODUCT_DIR}/tests/IndirectLoadIlInjector.cpp
	${JIT_PRODUCT_DIR}/tests/IndirectStoreIlInjector.cpp
	${JIT_PRODUCT_DIR}/tests/FooBarTest.cpp
	${JIT_PRODUCT_DIR}/tests/FooIlInjector.cpp
	${JIT_PRODUCT_DIR}/tests/LimitFileTest.cpp
	${JIT_PRODUCT_DIR}/tests/LogFileTest.cpp
	${JIT_PRODUCT_DIR}/tests/OMRTestEnv.cpp
	${JIT_PRODUCT_DIR}/tests/OptionSetTest.cpp
	${JIT_PRODUCT_DIR}/tests/OpCodesTest.cpp
	${JIT_PRODUCT_DIR}/tests/PPCOpCodesTest.cpp
	${JIT_PRODUCT_DIR}/tests/Qux2Test.cpp
	${JIT_PRODUCT_DIR}/tests/Qux2IlInjector.cpp
	${JIT_PRODUCT_DIR}/tests/SimplifierFoldAndTest.cpp
	${JIT_PRODUCT_DIR}/tests/S390OpCodesTest.cpp
	${JIT_PRODUCT_DIR}/tests/OptTestDriver.cpp
	${JIT_PRODUCT_DIR}/tests/TestDriver.cpp
	${JIT_PRODUCT_DIR}/tests/X86OpCodesTest.cpp
	${JIT_PRODUCT_DIR}/tests/main.cpp
	${JIT_PRODUCT_DIR}/compile/Method.cpp
	${JIT_PRODUCT_DIR}/control/TestJit.cpp
	${JIT_PRODUCT_DIR}/env/FrontEnd.cpp
	${JIT_PRODUCT_DIR}/ilgen/IlInjector.cpp
	${JIT_PRODUCT_DIR}/ilgen/TestIlGeneratorMethodDetails.cpp
	${JIT_PRODUCT_DIR}/optimizer/TestOptimizer.cpp
	${JIT_PRODUCT_DIR}/runtime/TestCodeCacheManager.cpp
	${JIT_PRODUCT_DIR}/runtime/TestJitConfig.cpp
)

#if target == x

list(APPEND JIT_PRODUCT_SOURCE_FILES
    ${JIT_PRODUCT_DIR}/x/codegen/Evaluator.cpp
    ${JIT_OMR_DIRTY_DIR}/x/env/OMRDebugEnv.cpp
    ${JIT_OMR_DIRTY_DIR}/x/env/OMRCPU.cpp
)

#endif



#include $(JIT_MAKE_DIR)/files/host/$(HOST_ARCH).mk
#include $(JIT_MAKE_DIR)/files/target/$(TARGET_ARCH).mk

#TODO needs to be in compiler detection wrapper
set( CMAKE_C_FLAGS "-pthread -fomit-frame-pointer -fasynchronous-unwind-tables -Wreturn-type -fno-dollars-in-identifiers")
set( CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++0x -fno-rtti -fno-threadsafe-statics -Wno-deprecated -Wno-enum-compare -Wno-invalid-offsetof -Wno-write-strings -Wno-narrowing")

add_definitions(
	-DTEST_PROJECT_SPECIFIC
)

#if 64bit
add_definitions(
	-DTR_HOST_64BIT
	-DBITVECTOR_64BIT
	-DTR_TARGET_64BIT
)
#endif

#if linux
add_definitions(-DLINUX)
#endif

#if x86
add_definitions(-DTR_HOST_X86)
add_definitions(-DTR_TARGET_X86)
#endif

add_definitions(
	-DSUPPORTS_THREAD_LOCAL
	-D_LONG_LONG
	-DBITVECTOR_BIT_NUMBERING_MSB
	-DJITTEST
	-DTEST_PROJECT_SPECIFIC
)


#dumb hack for gtest. Maybe not needed now that we link agains gtest?
include_directories(
	${PROJECT_SOURCE_DIR}/third_party/gtest-1.7.0/
	${PROJECT_SOURCE_DIR}/third_party/gtest-1.7.0/include
)

set(GENERATE_VERSION_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/build/scripts/generateVersion.pl)
set(TR_VERSION_FILE ${CMAKE_CURRENT_BINARY_DIR}/TRBuildName.cpp)
#version name hack

add_custom_command(
	OUTPUT ${TR_VERSION_FILE}
	COMMAND ${PERL_EXECUTABLE} ${GENERATE_VERSION_SCRIPT} tr.open.test > ${TR_VERSION_FILE}
)
#(PERL_PATH) $(GENERATE_VERSION_SCRIPT) $(PRODUCT_RELEASE) > $@
add_executable(compilertest

	${JIT_PRODUCT_BACKEND_SOURCES}
	${JIT_PRODUCT_SOURCE_FILES}
	${TR_VERSION_FILE}
)

target_link_libraries(compilertest
	tr_compile
	tr_control
	tr_env
	tr_infra
	tr_il
	tr_ras
	tr_codegen
	tr_optimizer
	tr_ilgen
	tr_ilbuilder
	tr_runtime
	tr_TargetCodegen
	dl
	omrGtest
)
