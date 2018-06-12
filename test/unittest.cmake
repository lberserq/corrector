execute_process(
    COMMAND ${CMAKE_CTEST_COMMAND} -R ${TEST}
    RESULT_VARIABLE RESULT)
if(NOT RESULT EQUAL 0)
    message(FATAL_ERROR "Test ${TEST} [${TARGET}] failed.")
endif()
