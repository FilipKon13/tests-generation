#include "doctest.h"
#include <sstream>

#include <testgen/manager.hpp>
using namespace test;
using namespace std;

class TestStream : public stringstream {
public:
    explicit TestStream(string const & name) {
        static_cast<stringstream &>(*this) << name;
    }
};

TEST_CASE("test_non_ocen") {
    OIOIOIManager<TestStream> manager("pro");
    manager.nextTest();

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
    manager.nextTest();

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

TEST_CASE("test_ocen-to-next-suite") {
    OIOIOIManager<TestStream> manager("pro", true);
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

TEST_CASE("test-get-filename") {
    OIOIOIManager<TestStream> manager("pro", true);
    manager.nextTest();

    CHECK(manager.getFilename() == "pro1ocen.in");
    manager.nextTest();
    CHECK(manager.getFilename() == "pro2ocen.in");
    manager.nextTest();
    CHECK(manager.getFilename() == "pro3ocen.in");

    manager.nextSuite();

    CHECK(manager.getFilename() == "pro1a.in");
    manager.nextTest();
    CHECK(manager.getFilename() == "pro1b.in");
    manager.nextTest();
    CHECK(manager.getFilename() == "pro1c.in");

    manager.nextSuite();

    CHECK(manager.getFilename() == "pro2a.in");
    manager.nextTest();
    CHECK(manager.getFilename() == "pro2b.in");
    manager.nextTest();
    CHECK(manager.getFilename() == "pro2c.in");

    manager.test(1, 2);
    CHECK(manager.getFilename() == "pro2a.in");
    manager.test(3, 2);
    CHECK(manager.getFilename() == "pro2c.in");
}
