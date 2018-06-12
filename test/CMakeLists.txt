cmake_minimum_required(VERSION 3.0)
include_directories(${spell_checker_SOURCE_DIR}/include ${spell_checker_SOURCE_DIR}/test)
link_directories(${spell_checker_SOURCE_DIR}/lib)
file(GLOB spell_checker_sources_tst
    "*.h"
    "*.cpp"
)
add_executable(spell_checker_tests ${spell_checker_sources_tst})
target_link_libraries(spell_checker_tests spell_checker "${GTEST_BOTH_LIBRARIES}" "${Boost_LIBRARIES}")
enable_testing()
add_test(NAME spell_checker_tests COMMAND spell_checker_tests)
add_custom_command(
        TARGET spell_checker_tests PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/test/en.txt
                ${CMAKE_CURRENT_BINARY_DIR}/en.txt)
add_custom_command(
        TARGET spell_checker_tests PRE_LINK
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/test/ru.txt
                ${CMAKE_CURRENT_BINARY_DIR}/ru.txt)
add_custom_command(TARGET spell_checker_tests POST_BUILD COMMAND ${CMAKE_COMMAND}
    -DTEST=spell_checker_tests
    -DTARGET=$<TARGET_FILE:spell_checker_tests>
    -P ${spell_checker_SOURCE_DIR}/test/unittest.cmake)
