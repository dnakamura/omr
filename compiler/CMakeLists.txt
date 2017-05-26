
Find_package(Perl)

if (NOT PERL_FOUND )
	message(FATAL_ERROR "Perl not found")
else()
	message(STATUS PERL = ${PERL_EXECUTABLE})
endif()

#TODO these should not be force-set
set(TR_TARGET_ARCH x)
set(TR_TARGET_SUBARCH amd64)

set(JIT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "TODO: DOCUMENT" FORCE)

#set(JIT_PRODUCT_DIR ../fvtest/compilertest)

set(CMAKE_INCLUDE_CURRENT_DIR OFF)

#this is dumb. should be set by product
set(MASM2GAS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../fvtest/compilertest/build/scripts/masm2gas.pl)

function(process_compiler_target tgtName)
	#we need to preprocess assembly files

endfunction()
function(masm2gas inpt)

endfunction()


#HACk there are alot of include directories here that shouldnt be
#Note for whatever reason, include dir paths is backward from the way you think
#ie paths at end are are added first
include_directories(
	${TR_TARGET_ARCH}/${TR_TARGET_SUBARCH}
	${TR_TARGET_ARCH}
	.
	../
)



#hack definitions in
add_definitions(
	-DBITVECTOR_BIT_NUMBERING_MSB
	-DUT_DIRECT_TRACE_REGISTRATION
	-DJITTEST

	-DTR_HOST_64BIT
	-DLINUX
	-DTR_HOST_X86
	-DTR_TARGET_X86
	-DTR_TARGET_64BIT
)

#TODO there is a much cleaner way of doing this
set(CMAKE_ASM-ATT_FLAGS
	-DBITVECTOR_BIT_NUMBERING_MSB
	-DUT_DIRECT_TRACE_REGISTRATION
	-DJITTEST
#	-DTEST_PROJECT_SPECIFIC

	-DTR_HOST_64BIT
	-DLINUX
	-DTR_HOST_X86
	-DTR_TARGET_X86
	-DTR_TARGET_64BIT
	)
#hack in compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-format -Wno-write-strings -Wno-deprecated -Wno-narrowing -m64 -fpic -fno-strict-aliasing -fno-rtti -Wno-int-to-pointer-cast")

if(OMR_ARCH_X86)
	add_subdirectory(x)
endif()

add_subdirectory(ras)
add_subdirectory(compile)
add_subdirectory(control)
add_subdirectory(env)
add_subdirectory(codegen)
add_subdirectory(infra)
add_subdirectory(il)
add_subdirectory(optimizer)
add_subdirectory(runtime)
add_subdirectory(ilgen)

#add_library(tr INTERFACE)
#target_link_libraries(tr
#	INTERFACE
#	tr_ras
#	tr_compile
#	tr_control
#	tr_env
#	tr_codegen
#	tr_infra
#	tr_il
#	tr_optimizer
#	tr_runtime
#	tr_ilgen
#)



#add_executable(dummy dummy.cpp)
#target_link_libraries(dummy tr)





#JIT_PRODUCT_BACKEND_SOURCES+=\




#JIT_PRODUCT_SOURCE_FILES+=\
    #$(JIT_PRODUCT_DIR)/ilgen/BinaryOpIlInjector.cpp \


#JIT_PRODUCT_SOURCE_FILES+=\

    #$(JIT_OMR_DIRTY_DIR)/x/env/OMRDebugEnv.cpp \
    #$(JIT_OMR_DIRTY_DIR)/x/env/OMRCPU.cpp

#include $(JIT_MAKE_DIR)/files/target/$(TARGET_SUBARCH).mk

#JIT_PRODUCT_SOURCE_FILES+=\


	 #$(JIT_PRODUCT_DIR)/control/TestJit.cpp \
	 
    #$(JIT_PRODUCT_DIR)/ilgen/ChildlessUnaryOpIlInjector.cpp \
    #$(JIT_PRODUCT_DIR)/ilgen/CmpBranchOpIlInjector.cpp \
    #$(JIT_PRODUCT_DIR)/ilgen/OpIlInjector.cpp \
    #$(JIT_PRODUCT_DIR)/ilgen/StoreOpIlInjector.cpp \
    #$(JIT_PRODUCT_DIR)/ilgen/TernaryOpIlInjector.cpp \
    #$(JIT_PRODUCT_DIR)/ilgen/UnaryOpIlInjector.cpp \
    #$(JIT_PRODUCT_DIR)/ilgen/IlInjector.cpp \
    #$(JIT_PRODUCT_DIR)/ilgen/TestIlGeneratorMethodDetails.cpp \
    
    #$(JIT_PRODUCT_DIR)/tests/BarIlInjector.cpp \
    #$(JIT_PRODUCT_DIR)/tests/BuilderTest.cpp \
    #$(JIT_PRODUCT_DIR)/tests/CallIlInjector.cpp \
    #$(JIT_PRODUCT_DIR)/tests/IndirectLoadIlInjector.cpp \
    #$(JIT_PRODUCT_DIR)/tests/IndirectStoreIlInjector.cpp \
    #$(JIT_PRODUCT_DIR)/tests/FooBarTest.cpp \
    #$(JIT_PRODUCT_DIR)/tests/FooIlInjector.cpp \
    #$(JIT_PRODUCT_DIR)/tests/LimitFileTest.cpp \
    #$(JIT_PRODUCT_DIR)/tests/LogFileTest.cpp \
    #$(JIT_PRODUCT_DIR)/tests/OMRTestEnv.cpp \
    #$(JIT_PRODUCT_DIR)/tests/OptionSetTest.cpp \
    #$(JIT_PRODUCT_DIR)/tests/OpCodesTest.cpp \
    #$(JIT_PRODUCT_DIR)/tests/PPCOpCodesTest.cpp \
    #$(JIT_PRODUCT_DIR)/tests/Qux2Test.cpp \
    #$(JIT_PRODUCT_DIR)/tests/Qux2IlInjector.cpp \
    #$(JIT_PRODUCT_DIR)/tests/SimplifierFoldAndTest.cpp \
    #$(JIT_PRODUCT_DIR)/tests/S390OpCodesTest.cpp \
    #$(JIT_PRODUCT_DIR)/tests/OptTestDriver.cpp \
    #$(JIT_PRODUCT_DIR)/tests/TestDriver.cpp \
    #$(JIT_PRODUCT_DIR)/tests/X86OpCodesTest.cpp \
    #$(JIT_PRODUCT_DIR)/tests/main.cpp \
    #$(JIT_PRODUCT_DIR)/compile/Method.cpp \
    #$(JIT_PRODUCT_DIR)/env/FrontEnd.cpp \

    #$(JIT_PRODUCT_DIR)/optimizer/TestOptimizer.cpp \
    #$(JIT_PRODUCT_DIR)/runtime/TestCodeCacheManager.cpp \
    #$(JIT_PRODUCT_DIR)/runtime/TestJitConfig.cpp



