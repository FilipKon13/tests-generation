cmake_minimum_required(VERSION 3.20)

project(testgen)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_program(CLANG_TIDY_BINARY NAMES "clang-tidy")
add_compile_options(-Wall -Wextra -Wpedantic) # TODO check for compiler
set(CLANG_TIDY_COMMAND "${CLANG_TIDY_BINARY}")

file(GLOB includes ${PROJECT_SOURCE_DIR}/include/testgen/*.hpp)
set(testgen_single ${PROJECT_SOURCE_DIR}/testgen.hpp)

add_custom_command(OUTPUT ${testgen_single}
    COMMAND python3 ${PROJECT_SOURCE_DIR}/scripts/merge.py ${PROJECT_SOURCE_DIR}/include ${testgen_single}
    DEPENDS ${includes} ${PROJECT_SOURCE_DIR}/scripts/merge.py)

add_custom_target(build_testgen DEPENDS ${testgen_single})

add_custom_target(clang-tidy COMMAND ${CLANG_TIDY_COMMAND} ${testgen_single} DEPENDS build_testgen)

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/main.o ${PROJECT_BINARY_DIR}/main.json
    COMMAND clang++ -std=c++17 -Wall -Wextra -ftime-trace -g -c ${testgen_single} -o ${PROJECT_BINARY_DIR}/main.o
    DEPENDS build_testgen)

add_custom_target(time-trace DEPENDS ${PROJECT_BINARY_DIR}/main.json)

enable_testing()
add_subdirectory(tests)
