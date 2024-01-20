cmake_minimum_required(VERSION 3.20)

project(testgen)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic) # TODO check for compiler

file(GLOB includes ${PROJECT_SOURCE_DIR}/include/testgen/*.hpp)
set(testgen_single ${PROJECT_SOURCE_DIR}/testgen.hpp)

add_custom_command(OUTPUT ${testgen_single}
    COMMAND python3 ${PROJECT_SOURCE_DIR}/scripts/merge.py ${PROJECT_SOURCE_DIR}/include ${testgen_single}
    DEPENDS ${includes} ${PROJECT_SOURCE_DIR}/scripts/merge.py)

add_custom_target(build_testgen DEPENDS ${testgen_single})

add_custom_target(clang-tidy COMMAND clang-tidy-16 ${testgen_single} DEPENDS build_testgen)
add_custom_target(clang-tidy-test COMMAND clang-tidy-16 -checks='-misc-use-anonymous-namespace, -readability-function-cognitive-complexity,-google-build-using-namespace, -*magic-numbers' -p ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/tests/src/unit-*.cpp)

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/main.o ${PROJECT_BINARY_DIR}/main.json
    COMMAND clang++ -std=c++17 -Wall -Wextra -ftime-trace -g -c ${testgen_single} -o ${PROJECT_BINARY_DIR}/main.o
    DEPENDS build_testgen)

add_custom_target(time-trace DEPENDS ${PROJECT_BINARY_DIR}/main.json)

add_custom_target(precommit DEPENDS clang-tidy clang-tidy-test)

enable_testing()
add_subdirectory(tests)
