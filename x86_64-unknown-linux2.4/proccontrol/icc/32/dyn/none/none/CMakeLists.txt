# CMakeLists for proccontrol/icc/32/dyn/none/none
set (CMAKE_C_FLAGS "${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}  -Di386_unknown_linux2_4 -Dm32_test -g -O0 -no-pie")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_FLAGS_RELWITHDEBINFO "")
set (CMAKE_C_COMPILER "${M_intel_cc_32}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU} -Di386_unknown_linux2_4 -Dm32_test -Xlinker -export-dynamic")
IF (NOT ${M_intel_cc_32} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_intel_cc_32}"
	"${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}  -Di386_unknown_linux2_4 -Dm32_test -g -O0 -no-pie"
	"${MUTATEE_LDFLAGS_GNU} -Di386_unknown_linux2_4 -Dm32_test -Xlinker -export-dynamic"
	"C"
	"intel_cc_32 32 dyn"
	MUTATEEintelcc3232dynC)

IF (MUTATEEintelcc3232dynC MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
if(TARGET pcontrol)
add_executable (pc_exec_targ.dyn_icc_32_none_none ${SOURCE_LIST_23})
foreach (f ${SOURCE_LIST_23})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(pc_exec_targ.dyn_icc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (pc_tls.dyn_icc_32_none_none ${SOURCE_LIST_60})
foreach (f ${SOURCE_LIST_60})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(pc_tls.dyn_icc_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_tls.dyn_icc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		pc_exec_targ.dyn_icc_32_none_none
		pc_tls.dyn_icc_32_none_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
ENDIF()
