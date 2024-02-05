file(GLOB files CONFIGURE_DEPENDS *.cpp)

foreach(file ${files})
    cmake_path(GET file FILENAME example_name)
    cmake_path(GET example_name STEM example_name)
    string(PREPEND example_name "example-")
    add_executable(${example_name} ${file})
    add_dependencies(example-all ${example_name})
    add_dependencies(${example_name} build_testgen)
    target_include_directories(${example_name} PRIVATE ${PROJECT_SOURCE_DIR})
endforeach()

