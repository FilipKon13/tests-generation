#include "doctest.h"
#include <testgen/output.hpp>
#include <sstream>
using namespace test;

TEST_CASE("test_empty") {
    std::stringstream stream;
    Output out(stream);

    out.dump_output();

    CHECK(stream.str() == "");
}

TEST_CASE("test_single_int") {
    std::stringstream stream;
    Output out(stream);

    out.dump_output(1);

    CHECK(stream.str() == "1\n");
}

TEST_CASE("test_single_space") {
    std::stringstream stream;
    Output out(stream);

    out.dump_output(space);

    CHECK(stream.str() == " ");
}

TEST_CASE("test_multiple_no_space") {
    std::stringstream stream;
    Output out(stream);

    out.dump_output(1, "22", std::string("333"));

    CHECK(stream.str() == "1\n22\n333\n");
}

TEST_CASE("test_multiple_space") {
    std::stringstream stream;
    Output out(stream);

    out.dump_output(1, space, "22", space, std::string("333"));

    CHECK(stream.str() == "1 22 333\n");
}

TEST_CASE("test_space_at_end") {
    std::stringstream stream;
    Output out(stream);

    out.dump_output(1, space, "22", std::string("333"), space);

    CHECK(stream.str() == "1 22\n333 ");
}