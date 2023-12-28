#include "doctest.h"
#include <testgen/testing.hpp>
#include <sstream>
#include <type_traits>

using namespace test;

struct TestManager {
    std::stringstream & stream_;
    gen_type gen_{0};
    TestManager(std::stringstream & stream) :
      stream_{stream} {}
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

TEST_CASE("test_usage_standard_types") {
    std::stringstream s;
    Testing<TestManager> test{s};

    test.nextSuite();
    test.print(1, 2, 'a');
    test.nextTest();
    test.nextTest();
    test.print("abc");

    CHECK(s.str() == "next suite\n1\n2\na\nnext test\nnext test\nabc\n");
}

TEST_CASE("test_generate") {
    std::stringstream s;
    Testing<TestManager> test{s};

    auto v1 = test(7);
    auto v2 = test(TestGenerating{});

    static_assert(std::is_same_v<int, decltype(v2)>);
    CHECK(v1 == 7);
    CHECK(v2 == 3);
}

TEST_CASE("test_generating_and_standard_types") {
    std::stringstream s;
    Testing<TestManager> test{s};

    test.nextSuite();
    test.print(2);
    test.print(TestGenerating{});
    test.print(1, space, TestGenerating{}, space, "abc");
    test.nextTest();
    test << 11 << TestGenerating{} << ' ' << "def";

    CHECK(s.str() == "next suite\n2\n3\n1 3 abc\nnext test\n113 def");
}

TEST_CASE("test-no-gen") {
    struct Testcase {};
    std::stringstream s;
    Testing<TestManager, Testcase> test{s};
}

TEST_CASE("test-gen") {
    struct Testcase {
        gen_type & gen;
    };
    std::stringstream s;
    Testing<TestManager, Testcase> test{s};
}

TEST_CASE("test-get-generator") {
    std::stringstream s;
    Testing<TestManager> test{s};
    [[maybe_unused]] auto gen = test.generator();
}

struct Testcase {
    int x;
    friend std::ostream & operator<<(std::ostream & s, Testcase const & t) {
        return s << t.x;
    }
};

TEST_CASE("check-assumption-empty") {
    Testcase T{2};
    std::stringstream s;
    Testing<TestManager, Testcase> test{s};
    test << T;
}

TEST_CASE("check-assumption-ok") {
    Testcase T{2};
    std::stringstream s;
    Testing<TestManager, Testcase> test{s};
    test.globalAssumption([](Testcase const & t) { return t.x == 2; });
    CHECK_NOTHROW(test << T);
}

TEST_CASE("check-assumption-bad") {
    Testcase T{2};
    std::stringstream s;
    Testing<TestManager, Testcase> test{s};
    test.globalAssumption([](Testcase const & t) { return t.x == 3; });
    CHECK_THROWS(test << T);
}

TEST_CASE("check-assumption-change-suite") {
    Testcase T{2};
    std::stringstream s;
    Testing<TestManager, Testcase> test{s};
    test.suiteAssumption([](Testcase const & t) { return t.x == 3; });
    SUBCASE("next-suite") {
        test.nextSuite();
        CHECK_NOTHROW(test << T);
    }
    SUBCASE("next-test") {
        test.nextTest();
        CHECK_THROWS(test << T);
    }
}

TEST_CASE("check-assumption-change-test") {
    Testcase T{2};
    std::stringstream s;
    Testing<TestManager, Testcase> test{s};
    test.testAssumption([](Testcase const & t) { return t.x == 3; });
    SUBCASE("next-suite") {
        test.nextSuite();
        CHECK_NOTHROW(test << T);
    }
    SUBCASE("next-test") {
        test.nextTest();
        CHECK_NOTHROW(test << T);
    }
}

TEST_CASE("test-generator-in-testcase") {
    std::stringstream s;
    Testing<TestManager, TestcaseBase> test{s};
    [[maybe_unused]] auto gen = test.nextSuite();
    static_assert(std::is_same_v<decltype(gen), TestcaseBase>);
}

TEST_CASE("test-default") {
    std::stringstream s;
    Testing<TestManager> test{s};
    [[maybe_unused]] auto gen = test.nextSuite();
    static_assert(std::is_same_v<decltype(gen), DummyTestcase>);
}