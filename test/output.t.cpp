#include <output.hpp>
#include <sstream>
#include "test.hpp"
using namespace test;

int main() {
    Output out;
    std::stringstream out1{}, out2{};

    out.set(&out1);
    out << 12 << '\n';
    out.set(&out2);
    out << 13 << '\n';
    out.set(&out1);
    out << 14 << '\n';
    out.set(&out2);
    out << 15 << '\n';

    assert(out1.str() == "12\n14\n");
    assert(out2.str() == "13\n15\n");
    
    TEST_OK();
    return 0;
}