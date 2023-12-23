#include <manager.hpp>
#include "test.hpp"
#include <sstream>
using namespace test;
using namespace std;


struct TestStream : stringstream {
    TestStream(string const & name) : stringstream{} {
        static_cast<stringstream&>(*this) << name;
    }
};

void test_non_ocen() {
    OIOIOIManager<TestStream> manager("pro");

    assert(manager.stream().str() == "pro1a.in");
    manager.nextTest();
    assert(manager.stream().str() == "pro1b.in");
    manager.nextTest();
    assert(manager.stream().str() == "pro1c.in");

    manager.nextSuite();

    assert(manager.stream().str() == "pro2a.in");
    manager.nextTest();
    assert(manager.stream().str() == "pro2b.in");
    manager.nextTest();
    assert(manager.stream().str() == "pro2c.in");

    manager.test(1, OCEN);

    assert(manager.stream().str() == "pro1ocen.in");
    manager.nextTest();
    assert(manager.stream().str() == "pro2ocen.in");
    manager.nextTest();
    assert(manager.stream().str() == "pro3ocen.in");

    manager.stream() << " test";

    manager.test(1, 2);
    assert(manager.stream().str() == "pro2a.in");
    manager.test(3, OCEN);
    assert(manager.stream().str() == "pro3ocen.in test");
}

void test_ocen() {
    OIOIOIManager<TestStream> manager("pro", true);

    assert(manager.stream().str() == "pro1ocen.in");
    manager.nextTest();
    assert(manager.stream().str() == "pro2ocen.in");
    manager.nextTest();
    assert(manager.stream().str() == "pro3ocen.in");

    manager.nextSuite();

    assert(manager.stream().str() == "pro1a.in");
    manager.nextTest();
    assert(manager.stream().str() == "pro1b.in");
    manager.nextTest();
    assert(manager.stream().str() == "pro1c.in");

    manager.nextSuite();

    assert(manager.stream().str() == "pro2a.in");
    manager.nextTest();
    assert(manager.stream().str() == "pro2b.in");
    manager.nextTest();
    assert(manager.stream().str() == "pro2c.in");

    manager.stream() << " test";

    manager.test(1, 2);
    assert(manager.stream().str() == "pro2a.in");
    manager.test(3, 2);
    assert(manager.stream().str() == "pro2c.in test");
}

int main() {
    test_non_ocen();
    test_ocen();

    TEST_OK();
    return 0;
}