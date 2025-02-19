# CMakeLists for proccontrol/icpc/32/dyn/none/none
set (CMAKE_CXX_FLAGS "${MUTATOR_DEFINES}  ${CXXFLAGS}  ${MUTATEE_CXXFLAGS_GNU}    -g -O0 -no-pie")
set (CMAKE_CXX_FLAGS_DEBUG "")
set (CMAKE_CXX_FLAGS_RELEASE "")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "")
set (CMAKE_CXX_COMPILER "${M_intel_CC}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic")
IF (NOT ${M_intel_CC} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_intel_CC}"
	"${MUTATOR_DEFINES}  ${CXXFLAGS}  ${MUTATEE_CXXFLAGS_GNU}    -g -O0 -no-pie"
	"${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic"
	"CXX"
	"intel_CC 32 dyn"
	MUTATEEintelCC32dynCXX)

IF (MUTATEEintelCC32dynCXX MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-linux2.4/srclists.cmake)
if(TARGET pcontrol)
add_executable (pc_exec_targ.dyn_icpc_32_none_none ${SOURCE_LIST_11})
foreach (f ${SOURCE_LIST_11})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE CXX)
	endif()
	endforeach()
set_target_properties(pc_exec_targ.dyn_icpc_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (pc_tls.dyn_icpc_32_none_none ${SOURCE_LIST_94})
foreach (f ${SOURCE_LIST_94})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE CXX)
	endif()
	endforeach()
set_target_properties(pc_tls.dyn_icpc_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_tls.dyn_icpc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		pc_exec_targ.dyn_icpc_32_none_none
		pc_tls.dyn_icpc_32_none_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
ENDIF()
