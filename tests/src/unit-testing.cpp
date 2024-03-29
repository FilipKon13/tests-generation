#include <doctest.h>

#include <sstream>
#include <type_traits>

#include <testgen/testing.hpp>
using namespace test;

class TestManager {
    std::reference_wrapper<std::stringstream> m_stream;
    gen_type m_gen{0};

public:
    explicit TestManager(std::stringstream & stream) :
      m_stream{stream} {}
    void nextSuite() {
        m_stream.get() << "next suite\n";
    }
    void nextTest() {
        m_stream.get() << "next test\n";
    }
    std::ostream & stream() {
        return m_stream.get();
    }
    gen_type & generator() {
        return m_gen;
    }
    static char const * getFilename() {
        return "mock";
    }
};

class TestGenerating : public Generating<int> {
public:
    int generate(gen_type & /* unused */) const override {
        return 3;
    }
};

TEST_CASE("test_usage_standard_types") {
    std::stringstream s;
    Testing<TestManager> test{s};

    test.nextSuite();
    test.getTest();
    test << 1 << '\n'
         << 2 << '\n'
         << 'a' << '\n';
    test.getTest();
    test.getTest();
    test << "abc\n";

    CHECK(s.str() == "next suite\nnext test\n1\n2\na\nnext test\nnext test\nabc\n");
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
    test.getTest();
    test << 2 << '\n'
         << TestGenerating{} << '\n'
         << 1 << ' ' << TestGenerating{} << ' ' << "abc" << '\n';
    test.getTest();
    test << 11 << TestGenerating{} << ' ' << "def";

    CHECK(s.str() == "next suite\nnext test\n2\n3\n1 3 abc\nnext test\n113 def");
}

class Testcase {
public:
    int x;
    friend std::ostream & operator<<(std::ostream & s, Testcase const & t) {
        return s << t.x;
    }
};

TEST_CASE("check-assumption-empty") {
    Testcase const T{2};
    std::stringstream s;
    Testing<TestManager, Testcase> test{s};
    test << T;
}

TEST_CASE("check-assumption-ok") {
    Testcase const T{2};
    std::stringstream s;
    Testing<TestManager, Testcase> test{s};
    test.assumptionGlobal([](Testcase const & t) { return t.x == 2; });
    CHECK_NOTHROW(test << T);
}

DEATH_TEST("check-assumption-bad") {
    Testcase const T{2};
    std::stringstream s;
    Testing<TestManager, Testcase> test{s};
    test.assumptionGlobal([](Testcase const & t) { return t.x == 3; });
    CHECK_DEATH(test << T);
}

DEATH_TEST("check-assumption-change-suite") {
    Testcase const T{2};
    std::stringstream s;
    Testing<TestManager, Testcase> test{s};
    test.assumptionSuite([](Testcase const & t) { return t.x == 3; });
    SUBCASE("next-suite") {
        test.nextSuite();
        CHECK_NOTHROW(test << T);
    }
    SUBCASE("next-test") {
        test.getTest();
        CHECK_DEATH(test << T);
    }
}

TEST_CASE("check-assumption-change-test") {
    Testcase const T{2};
    std::stringstream s;
    Testing<TestManager, Testcase> test{s};
    test.assumptionTest([](Testcase const & t) { return t.x == 3; });
    SUBCASE("next-suite") {
        test.nextSuite();
        CHECK_NOTHROW(test << T);
    }
    SUBCASE("next-test") {
        test.getTest();
        CHECK_NOTHROW(test << T);
    }
}

TEST_CASE("test-return-default-testcase-instance") {
    std::stringstream s;
    Testing<TestManager> test{s};
    [[maybe_unused]] auto gen = test.getTest();
    static_assert(std::is_same_v<decltype(gen), DummyTestcase>);
}

TEST_CASE("test-return-default-testcase-instance") {
    class TestTestcase {};
    std::stringstream s;
    Testing<TestManager, TestTestcase> test{s};
    [[maybe_unused]] auto gen = test.getTest();
    static_assert(std::is_same_v<decltype(gen), TestTestcase>);
}

TEST_CASE("test-rng-utilities") {
    std::stringstream s;
    Testing<TestManager> const test{s};
    using T = std::remove_cv_t<decltype(test)>;
    static_assert(std::is_base_of_v<RngUtilities<T>, T>);
}
