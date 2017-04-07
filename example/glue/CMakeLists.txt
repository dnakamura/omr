add_library(omrglue STATIC
	CollectorLanguageInterfaceImpl.cpp
	CompactSchemeFixupObject.cpp
	EnvironmentDelegate.cpp
	FrequentObjectsStats.cpp
	LanguageVMGlue.c
	ObjectIterator.cpp
	ObjectModelDelegate.cpp
	Profiling.c
	StartupManagerImpl.cpp
	UtilGlue.c
	VerboseManagerImpl.cpp
	omrExampleVM.cpp
)

target_include_directories(omrglue PUBLIC . ..)
target_link_libraries(omrglue PRIVATE omrgc)
