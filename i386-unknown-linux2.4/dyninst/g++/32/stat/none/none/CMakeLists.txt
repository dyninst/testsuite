# CMakeLists for dyninst/g++/32/stat/none/none
set (CMAKE_CXX_FLAGS "${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}   -g -O0 -no-pie")
set (CMAKE_CXX_FLAGS_DEBUG "")
set (CMAKE_CXX_FLAGS_RELEASE "")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "")
set (CMAKE_CXX_COMPILER "${M_gnu_cxx}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/dyninst")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_SKIP_INSTALL_RPATH TRUE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -static")
IF (NOT ${M_gnu_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cxx}"
	"${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}   -g -O0 -no-pie"
	"${MUTATEE_LDFLAGS_GNU}  -static"
	"CXX"
	"gnu_cxx 32 stat"
	MUTATEEgnucxx32statCXX)

IF (MUTATEEgnucxx32statCXX MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-linux2.4/srclists.cmake)
if(TARGET dyninstAPI)
add_executable (dyninst_cxx_group_test.stat_g++_32_none_none ${SOURCE_LIST_84})
foreach (f ${SOURCE_LIST_84})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE CXX)
	endif()
	endforeach()
set_target_properties(dyninst_cxx_group_test.stat_g++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (dyninst_group_test.stat_g++_32_none_none ${SOURCE_LIST_2})
foreach (f ${SOURCE_LIST_2})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE CXX)
	endif()
	endforeach()
set_target_properties(dyninst_group_test.stat_g++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (dyninst_group_test.stat_g++_32_none_none ${CMAKE_DL_LIBS})
add_executable (test1_14.stat_g++_32_none_none ${SOURCE_LIST_62})
foreach (f ${SOURCE_LIST_62})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE CXX)
	endif()
	endforeach()
set_target_properties(test1_14.stat_g++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test1_29.stat_g++_32_none_none ${SOURCE_LIST_50})
foreach (f ${SOURCE_LIST_50})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE CXX)
	endif()
	endforeach()
set_target_properties(test1_29.stat_g++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)


INSTALL (TARGETS
		dyninst_cxx_group_test.stat_g++_32_none_none
		dyninst_group_test.stat_g++_32_none_none
		test1_14.stat_g++_32_none_none
		test1_29.stat_g++_32_none_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
ENDIF()
