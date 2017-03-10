set(ASM_DIALECT "-ZOS")
set(CMAKE_ASM${ASM_DIALECT}_COMPILER_LIST xlc)
include(CMakeDetermineASMCompiler)
set(ASM_DIALECT)
