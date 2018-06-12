cmake_minimum_required(VERSION 3.0)
include_directories(${spell_checker_SOURCE_DIR}/include ${spell_checker_SOURCE_DIR}/lib)
file(GLOB spell_checker_sources
    "*.h"
    "*.cpp"
)

add_library(spell_checker SHARED ${spell_checker_sources})
target_link_libraries(spell_checker "${Boost_LIBRARIES}")
