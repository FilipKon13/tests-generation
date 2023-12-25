cmake_minimum_required(VERSION 3.20)
project(testgen)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_program(CLANG_TIDY_BINARY NAMES "clang-tidy")
set(CLANG_TIDY_COMMAND "${CLANG_TIDY_BINARY}")

file(GLOB includes ${PROJECT_SOURCE_DIR}/include/testgen/*.hpp)
set(testgen_single ${PROJECT_SOURCE_DIR}/testgen.hpp)

add_custom_command(OUTPUT ${testgen_single}
    COMMAND python3 ${PROJECT_SOURCE_DIR}/scripts/merge.py ${PROJECT_SOURCE_DIR}/include ${testgen_single}
    DEPENDS ${includes})

add_custom_target(build_testgen DEPENDS ${testgen_single})

add_custom_target(clang-tidy COMMAND ${CLANG_TIDY_COMMAND} ${testgen_single} DEPENDS build_testgen)

enable_testing()
add_subdirectory(tests)
