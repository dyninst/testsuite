# CMakeLists for dyninst/gcc/64/dyn/none/none
set (CMAKE_C_FLAGS "${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}   -g -O0 -no-pie")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_FLAGS_RELWITHDEBINFO "")
set (CMAKE_C_COMPILER "${M_gnu_cc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/dyninst")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
IF (NOT ${M_gnu_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cc}"
	"${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}   -g -O0 -no-pie"
	"${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic"
	"C"
	"gnu_cc 64 dyn"
	MUTATEEgnucc64dynC)

IF (MUTATEEgnucc64dynC MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
if(TARGET dyninstAPI)
add_executable (amd64_7_arg_call.dyn_gcc_64_none_none ${SOURCE_LIST_40})
foreach (f ${SOURCE_LIST_40})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(amd64_7_arg_call.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (dyninst_group_test.dyn_gcc_64_none_none ${SOURCE_LIST_6})
foreach (f ${SOURCE_LIST_6})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(dyninst_group_test.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (dyninst_group_test.dyn_gcc_64_none_none ${CMAKE_DL_LIBS})
add_executable (init_fini_callback.dyn_gcc_64_none_none ${SOURCE_LIST_56})
foreach (f ${SOURCE_LIST_56})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(init_fini_callback.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (init_fini_callback.dyn_gcc_64_none_none ${CMAKE_DL_LIBS})
add_executable (test1_12.dyn_gcc_64_none_none ${SOURCE_LIST_70})
foreach (f ${SOURCE_LIST_70})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test1_12.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_14.dyn_gcc_64_none_none ${SOURCE_LIST_45})
foreach (f ${SOURCE_LIST_45})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test1_14.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_19.dyn_gcc_64_none_none ${SOURCE_LIST_21})
foreach (f ${SOURCE_LIST_21})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test1_19.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_29.dyn_gcc_64_none_none ${SOURCE_LIST_4})
foreach (f ${SOURCE_LIST_4})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test1_29.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_40.dyn_gcc_64_none_none ${SOURCE_LIST_36})
foreach (f ${SOURCE_LIST_36})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test1_40.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_41.dyn_gcc_64_none_none ${SOURCE_LIST_38})
foreach (f ${SOURCE_LIST_38})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test1_41.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test2_14.dyn_gcc_64_none_none ${SOURCE_LIST_57})
foreach (f ${SOURCE_LIST_57})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test2_14.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test2_6.dyn_gcc_64_none_none ${SOURCE_LIST_1})
foreach (f ${SOURCE_LIST_1})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test2_6.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test2_6.dyn_gcc_64_none_none ${CMAKE_DL_LIBS})
add_executable (test2_8.dyn_gcc_64_none_none ${SOURCE_LIST_2})
foreach (f ${SOURCE_LIST_2})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test2_8.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_1.dyn_gcc_64_none_none ${SOURCE_LIST_30})
foreach (f ${SOURCE_LIST_30})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test3_1.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_2.dyn_gcc_64_none_none ${SOURCE_LIST_35})
foreach (f ${SOURCE_LIST_35})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test3_2.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_3.dyn_gcc_64_none_none ${SOURCE_LIST_52})
foreach (f ${SOURCE_LIST_52})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test3_3.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_4.dyn_gcc_64_none_none ${SOURCE_LIST_61})
foreach (f ${SOURCE_LIST_61})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test3_4.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_5.dyn_gcc_64_none_none ${SOURCE_LIST_47})
foreach (f ${SOURCE_LIST_47})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test3_5.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_6.dyn_gcc_64_none_none ${SOURCE_LIST_50})
foreach (f ${SOURCE_LIST_50})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test3_6.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_7.dyn_gcc_64_none_none ${SOURCE_LIST_26})
foreach (f ${SOURCE_LIST_26})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test3_7.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_1.dyn_gcc_64_none_none ${SOURCE_LIST_3})
foreach (f ${SOURCE_LIST_3})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test4_1.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_2.dyn_gcc_64_none_none ${SOURCE_LIST_69})
foreach (f ${SOURCE_LIST_69})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test4_2.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_3.dyn_gcc_64_none_none ${SOURCE_LIST_64})
foreach (f ${SOURCE_LIST_64})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test4_3.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_3b.dyn_gcc_64_none_none ${SOURCE_LIST_71})
foreach (f ${SOURCE_LIST_71})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test4_3b.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_4.dyn_gcc_64_none_none ${SOURCE_LIST_20})
foreach (f ${SOURCE_LIST_20})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test4_4.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_4b.dyn_gcc_64_none_none ${SOURCE_LIST_58})
foreach (f ${SOURCE_LIST_58})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test4_4b.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_callback_1.dyn_gcc_64_none_none ${SOURCE_LIST_14})
foreach (f ${SOURCE_LIST_14})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_callback_1.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_callback_2.dyn_gcc_64_none_none ${SOURCE_LIST_28})
foreach (f ${SOURCE_LIST_28})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_callback_2.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_10.dyn_gcc_64_none_none ${SOURCE_LIST_48})
foreach (f ${SOURCE_LIST_48})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_fork_10.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_11.dyn_gcc_64_none_none ${SOURCE_LIST_17})
foreach (f ${SOURCE_LIST_17})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_fork_11.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_12.dyn_gcc_64_none_none ${SOURCE_LIST_25})
foreach (f ${SOURCE_LIST_25})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_fork_12.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_13.dyn_gcc_64_none_none ${SOURCE_LIST_10})
foreach (f ${SOURCE_LIST_10})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_fork_13.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_14.dyn_gcc_64_none_none ${SOURCE_LIST_18})
foreach (f ${SOURCE_LIST_18})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_fork_14.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_5.dyn_gcc_64_none_none ${SOURCE_LIST_65})
foreach (f ${SOURCE_LIST_65})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_fork_5.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_6.dyn_gcc_64_none_none ${SOURCE_LIST_49})
foreach (f ${SOURCE_LIST_49})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_fork_6.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_7.dyn_gcc_64_none_none ${SOURCE_LIST_22})
foreach (f ${SOURCE_LIST_22})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_fork_7.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_8.dyn_gcc_64_none_none ${SOURCE_LIST_15})
foreach (f ${SOURCE_LIST_15})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_fork_8.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_9.dyn_gcc_64_none_none ${SOURCE_LIST_37})
foreach (f ${SOURCE_LIST_37})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_fork_9.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_reloc.dyn_gcc_64_none_none ${SOURCE_LIST_24})
foreach (f ${SOURCE_LIST_24})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_reloc.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_snip_remove.dyn_gcc_64_none_none ${SOURCE_LIST_51})
foreach (f ${SOURCE_LIST_51})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_snip_remove.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_stack_1.dyn_gcc_64_none_none ${SOURCE_LIST_44})
foreach (f ${SOURCE_LIST_44})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_stack_1.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_stack_2.dyn_gcc_64_none_none ${SOURCE_LIST_59})
foreach (f ${SOURCE_LIST_59})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_stack_2.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_stack_3.dyn_gcc_64_none_none ${SOURCE_LIST_54})
foreach (f ${SOURCE_LIST_54})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_stack_3.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_stack_4.dyn_gcc_64_none_none ${SOURCE_LIST_67})
foreach (f ${SOURCE_LIST_67})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_stack_4.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_thread_1.dyn_gcc_64_none_none ${SOURCE_LIST_32})
foreach (f ${SOURCE_LIST_32})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_thread_1.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_1.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_2.dyn_gcc_64_none_none ${SOURCE_LIST_33})
foreach (f ${SOURCE_LIST_33})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_thread_2.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_2.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_3.dyn_gcc_64_none_none ${SOURCE_LIST_13})
foreach (f ${SOURCE_LIST_13})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_thread_3.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_3.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_5.dyn_gcc_64_none_none ${SOURCE_LIST_12})
foreach (f ${SOURCE_LIST_12})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_thread_5.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_5.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_6.dyn_gcc_64_none_none ${SOURCE_LIST_62})
foreach (f ${SOURCE_LIST_62})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_thread_6.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_6.dyn_gcc_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_7.dyn_gcc_64_none_none ${SOURCE_LIST_68})
foreach (f ${SOURCE_LIST_68})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_thread_7.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_7.dyn_gcc_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_8.dyn_gcc_64_none_none ${SOURCE_LIST_8})
foreach (f ${SOURCE_LIST_8})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test_thread_8.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_8.dyn_gcc_64_none_none ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		amd64_7_arg_call.dyn_gcc_64_none_none
		dyninst_group_test.dyn_gcc_64_none_none
		init_fini_callback.dyn_gcc_64_none_none
		test1_12.dyn_gcc_64_none_none
		test1_14.dyn_gcc_64_none_none
		test1_19.dyn_gcc_64_none_none
		test1_29.dyn_gcc_64_none_none
		test1_40.dyn_gcc_64_none_none
		test1_41.dyn_gcc_64_none_none
		test2_14.dyn_gcc_64_none_none
		test2_6.dyn_gcc_64_none_none
		test2_8.dyn_gcc_64_none_none
		test3_1.dyn_gcc_64_none_none
		test3_2.dyn_gcc_64_none_none
		test3_3.dyn_gcc_64_none_none
		test3_4.dyn_gcc_64_none_none
		test3_5.dyn_gcc_64_none_none
		test3_6.dyn_gcc_64_none_none
		test3_7.dyn_gcc_64_none_none
		test4_1.dyn_gcc_64_none_none
		test4_2.dyn_gcc_64_none_none
		test4_3.dyn_gcc_64_none_none
		test4_3b.dyn_gcc_64_none_none
		test4_4.dyn_gcc_64_none_none
		test4_4b.dyn_gcc_64_none_none
		test_callback_1.dyn_gcc_64_none_none
		test_callback_2.dyn_gcc_64_none_none
		test_fork_10.dyn_gcc_64_none_none
		test_fork_11.dyn_gcc_64_none_none
		test_fork_12.dyn_gcc_64_none_none
		test_fork_13.dyn_gcc_64_none_none
		test_fork_14.dyn_gcc_64_none_none
		test_fork_5.dyn_gcc_64_none_none
		test_fork_6.dyn_gcc_64_none_none
		test_fork_7.dyn_gcc_64_none_none
		test_fork_8.dyn_gcc_64_none_none
		test_fork_9.dyn_gcc_64_none_none
		test_reloc.dyn_gcc_64_none_none
		test_snip_remove.dyn_gcc_64_none_none
		test_stack_1.dyn_gcc_64_none_none
		test_stack_2.dyn_gcc_64_none_none
		test_stack_3.dyn_gcc_64_none_none
		test_stack_4.dyn_gcc_64_none_none
		test_thread_1.dyn_gcc_64_none_none
		test_thread_2.dyn_gcc_64_none_none
		test_thread_3.dyn_gcc_64_none_none
		test_thread_5.dyn_gcc_64_none_none
		test_thread_6.dyn_gcc_64_none_none
		test_thread_7.dyn_gcc_64_none_none
		test_thread_8.dyn_gcc_64_none_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
ENDIF()
