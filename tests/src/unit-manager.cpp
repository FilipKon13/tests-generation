#include "doctest.h"
#include <manager.hpp>
#include <sstream>
using namespace test;
using namespace std;


struct TestStream : stringstream {
    TestStream(string const & name) : stringstream{} {
        static_cast<stringstream&>(*this) << name;
    }
};

TEST_CASE("test_non_ocen") {
    OIOIOIManager<TestStream> manager("pro");

    CHECK(manager.stream().str() == "pro1a.in");
    manager.nextTest();
    CHECK(manager.stream().str() == "pro1b.in");
    manager.nextTest();
    CHECK(manager.stream().str() == "pro1c.in");

    manager.nextSuite();

    CHECK(manager.stream().str() == "pro2a.in");
    manager.nextTest();
    CHECK(manager.stream().str() == "pro2b.in");
    manager.nextTest();
    CHECK(manager.stream().str() == "pro2c.in");

    manager.test(1, OCEN);

    CHECK(manager.stream().str() == "pro1ocen.in");
    manager.nextTest();
    CHECK(manager.stream().str() == "pro2ocen.in");
    manager.nextTest();
    CHECK(manager.stream().str() == "pro3ocen.in");

    manager.stream() << " test";

    manager.test(1, 2);
    CHECK(manager.stream().str() == "pro2a.in");
    manager.test(3, OCEN);
    CHECK(manager.stream().str() == "pro3ocen.in test");
}

TEST_CASE("test_ocen") {
    OIOIOIManager<TestStream> manager("pro", true);

    CHECK(manager.stream().str() == "pro1ocen.in");
    manager.nextTest();
    CHECK(manager.stream().str() == "pro2ocen.in");
    manager.nextTest();
    CHECK(manager.stream().str() == "pro3ocen.in");

    manager.nextSuite();

    CHECK(manager.stream().str() == "pro1a.in");
    manager.nextTest();
    CHECK(manager.stream().str() == "pro1b.in");
    manager.nextTest();
    CHECK(manager.stream().str() == "pro1c.in");

    manager.nextSuite();

    CHECK(manager.stream().str() == "pro2a.in");
    manager.nextTest();
    CHECK(manager.stream().str() == "pro2b.in");
    manager.nextTest();
    CHECK(manager.stream().str() == "pro2c.in");

    manager.stream() << " test";

    manager.test(1, 2);
    CHECK(manager.stream().str() == "pro2a.in");
    manager.test(3, 2);
    CHECK(manager.stream().str() == "pro2c.in test");
}