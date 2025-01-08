project(simpleres_testing)
enable_testing()

# Macro from https://bertvandenbroucke.netlify.app/2019/12/12/unit-testing-with-ctest/
macro(add_unit_test)
    set(options PARALLEL)
    set(oneValueArgs NAME)
    set(multiValueArgs SOURCES LIBS)
    cmake_parse_arguments(TEST "${options}" "${oneValueArgs}"
                               "${multiValueArgs}" ${ARGN})
    message(STATUS "generating ${TEST_NAME}")
    add_executable(${TEST_NAME} EXCLUDE_FROM_ALL ${TEST_SOURCES})
    set_target_properties(${TEST_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                          ${PROJECT_BINARY_DIR}/rundir/test)
    target_link_libraries(${TEST_NAME} ${TEST_LIBS})

    if(TEST_PARALLEL AND HAVE_MPI)
      set(TESTCOMMAND ${MPIEXEC})
      set(TESTARGS ${MPIEXEC_NUMPROC_FLAG} 3 ${MPIEXEC_PREFLAGS}
                   "./${TEST_NAME}" ${MPIEXEC_POSTFLAGS})
      set(TESTCOMMAND ${TESTCOMMAND} ${TESTARGS})
    else(TEST_PARALLEL AND HAVE_MPI)
      set(TESTCOMMAND ${TEST_NAME})
    endif(TEST_PARALLEL AND HAVE_MPI)
    add_test(NAME ${TEST_NAME}
             WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/rundir/test
             COMMAND ${TESTCOMMAND})

    set(TESTNAMES ${TESTNAMES} ${TEST_NAME})
endmacro(add_unit_test)


add_unit_test(
	NAME pack_header_test
	SOURCES pack_header_testing.c
	LIBS smpr::smpr
)


add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
                        DEPENDS ${TESTNAMES})
