#include <testing.hpp>
#include <sstream>
#include "test.hpp"

using namespace test;

struct TestManager {
    std::stringstream & stream;
    testcase tcase;
    TestManager(std::stringstream & stream) : stream{stream}, tcase{&stream, gen_type{}} {}
    void nextSuite() {
        stream << "next suite\n";
    }
    void nextTest() {
        stream << "next test\n";
    }
    testcase& current() {
        return tcase;
    }
};

int main() {
    std::stringstream s;
    Testing<TestManager> test{s};
    
    test.nextSuite();
    test.print(1,2,'a');
    test.nextTest();
    test.nextTest();
    test.print("abc");

    assert(s.str() == "next suite\n1\n2\na\nnext test\nnext test\nabc\n");

    TEST_OK();
    return 0;
}