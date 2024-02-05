add_library (test_main OBJECT src/unit.cpp)
target_include_directories(test_main PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_definitions(test_main PUBLIC DOCTEST_CONFIG_SUPER_FAST_ASSERTS TESTRUN)
file(GLOB files CONFIGURE_DEPENDS src/unit-*.cpp)

foreach(file ${files})
    cmake_path(GET file FILENAME test_name)
    cmake_path(GET test_name STEM test_name)
    string(PREPEND test_name "test-")
    add_executable(${test_name} ${file})
    target_link_libraries(${test_name} test_main)
    target_compile_options(${test_name} PRIVATE -Wall -Wextra -Wpedantic) # check from TODO
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
