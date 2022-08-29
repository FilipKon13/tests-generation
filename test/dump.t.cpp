#include <output.hpp>
#include "test.hpp"
#include <sstream>
using namespace test;

void test_empty() {
    std::stringstream stream;
    Output out(stream);
    
    out.dump_output();

    assert(stream.str() == "");
}

void test_single_int() {
    std::stringstream stream;
    Output out(stream);
    
    out.dump_output(1);

    assert(stream.str() == "1\n");
}

void test_single_space() {
    std::stringstream stream;
    Output out(stream);
    
    out.dump_output(space);

    assert(stream.str() == " ");
}

void test_multiple_no_space() {
    std::stringstream stream;
    Output out(stream);
    
    out.dump_output(1, "22", std::string("333"));

    assert(stream.str() == "1\n22\n333\n");
}

void test_multiple_space() {
    std::stringstream stream;
    Output out(stream);
    
    out.dump_output(1, space, "22", space, std::string("333"));

    assert(stream.str() == "1 22 333\n");
}

void test_space_at_end() {
    std::stringstream stream;
    Output out(stream);
    
    out.dump_output(1, space, "22", std::string("333"), space);

    assert(stream.str() == "1 22\n333 ");
}

int main() {
    test_empty();
    test_single_int();
    test_single_space();
    test_multiple_no_space();
    test_multiple_space();
    TEST_OK();
}