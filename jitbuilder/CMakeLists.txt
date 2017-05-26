
#TODO these should not be force-set
set(TR_TARGET_ARCH x)
set(TR_TARGET_SUBARCH amd64)

#this is dumb. should be set by product
#TODO should be other masm to gas
set(MASM2GAS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../fvtest/compilertest/build/scripts/masm2gas.pl)

set(HOST_ARCH x)
set(HOST_SUBARCH amd64)


set(TARGET_ARCH ${HOST_ARCH})
set(TARGET_SUBARCH ${HOST_SUBARCH})

set(JIT_PRODUCT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(JIT_OMR_DIRTY_DIR ${CMAKE_SOURCE_DIR}/compiler)

#TODO needs to be in compiler detection wrapper
set( CMAKE_C_FLAGS "-pthread -fomit-frame-pointer -fasynchronous-unwind-tables -Wreturn-type -fno-dollars-in-identifiers")
set( CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++0x -fno-rtti -fno-threadsafe-statics -Wno-deprecated -Wno-enum-compare -Wno-invalid-offsetof -Wno-write-strings -Wno-narrowing")


#HACk there are alot of include directories here that shouldnt be
#Note for whatever reason, include dir paths is backward from the way you think
#ie paths at end are are added first
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


add_definitions(
	-DBITVECTOR_BIT_NUMBERING_MSB
	-DJITTEST
	-DJITBUILDER_SPECIFIC

	-DSUPPORTS_THREAD_LOCAL
	-D_LONG_LONG
	-DBITVECTOR_BIT_NUMBERING_MSB
)

#if 64bit
add_definitions(
	-DTR_HOST_64BIT
	-DBITVECTOR_64BIT
	-DTR_TARGET_64BIT
)
#endif


#if x86
add_definitions(-DTR_HOST_X86)
add_definitions(-DTR_TARGET_X86)
#endif


add_library(jitbuilder STATIC

	#if target ==x
	${JIT_PRODUCT_DIR}/x/codegen/Evaluator.cpp

	#if host == x
	#TODO: need asm
	#$(JIT_PRODUCT_DIR)/x/runtime/AsmUtil64.asm


	compile/Method.cpp
	control/Jit.cpp
	env/FrontEnd.cpp
	ilgen/JBIlGeneratorMethodDetails.cpp
	optimizer/JBOptimizer.cpp
	runtime/JBCodeCacheManager.cpp
	runtime/JBJitConfig.cpp
)

target_link_libraries(jitbuilder PUBLIC
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
)
