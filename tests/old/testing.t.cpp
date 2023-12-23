#include <testing.hpp>
#include <sstream>
#include "test.hpp"

using namespace test;

struct TestManager {
    std::stringstream & stream_;
    gen_type gen_{};
    TestManager(std::stringstream & stream) : stream_{stream} {}
    void nextSuite() {
        stream_ << "next suite\n";
    }
    void nextTest() {
        stream_ << "next test\n";
    }
    std::ostream & stream() {
        return stream_;
    }
    gen_type & generator() {
        return gen_;
    }
};

struct TestGenerating : public Generating<int> {
    int generate(gen_type &) const override {
        return 3;
    }
};

void test_usage_standard_types() {
    std::stringstream s;
    Testing<TestManager> test{s};
    
    test.nextSuite();
    test.print(1,2,'a');
    test.nextTest();
    test.nextTest();
    test.print("abc");

    assert(s.str() == "next suite\n1\n2\na\nnext test\nnext test\nabc\n");
}

void test_generate() {
    std::stringstream s;
    Testing<TestManager> test{s};
    
    auto v1 = test(7);
    auto v2 = test(TestGenerating{});

    assert(v1 == 7);
    static_assert(std::is_same_v<int, decltype(v2)>);
    assert(v2 == 3);
}

void test_generating_and_standard_types() {
    std::stringstream s;
    Testing<TestManager> test{s};

    test.nextSuite();
    test.print(2);
    test.print(TestGenerating{});
    test.print(1, space, TestGenerating{}, space, "abc");
    test.nextTest();
    test << 11 << TestGenerating{} << ' ' << "def";

    assert(s.str() == "next suite\n2\n3\n1 3 abc\nnext test\n113 def");
}

int main() {
    test_usage_standard_types();
    test_generate();
    test_generating_and_standard_types();

    TEST_OK();
    return 0;
}