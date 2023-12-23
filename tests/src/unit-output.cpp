#include "doctest.h"
#include <output.hpp>
#include <sstream>
using namespace test;

TEST_CASE("output-change-test") {
    Output out;
    std::stringstream out1{}, out2{};

    out.set(out1);
    out << 12 << '\n';
    out.set(out2);
    out << 13 << '\n';
    out.set(out1);
    out << 14 << '\n';
    out.set(out2);
    out << 15 << '\n';

    CHECK(out1.str() == "12\n14\n");
    CHECK(out2.str() == "13\n15\n");
}