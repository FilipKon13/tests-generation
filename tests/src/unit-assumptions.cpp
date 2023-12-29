#include "doctest.h"

#include <testgen/assumptions.hpp>
using namespace std;
using namespace test;

using Checker = AssumptionManager<int>;

TEST_CASE("test-empty") {
    Checker checker;
    CHECK(checker.check(7));
}

TEST_CASE("test-single-ok") {
    Checker checker;
    SUBCASE("global") {
        checker.setGlobal([](auto) { return true; });
        CHECK(checker.check(7));
    }
    SUBCASE("suite") {
        checker.setSuite([](auto) { return true; });
        CHECK(checker.check(7));
    }
    SUBCASE("test") {
        checker.setTest([](auto) { return true; });
        CHECK(checker.check(7));
    }
}

TEST_CASE("test-single-bad") {
    Checker checker;
    SUBCASE("global") {
        checker.setGlobal([](auto) { return false; });
        CHECK_FALSE(checker.check(7));
    }
    SUBCASE("suite") {
        checker.setSuite([](auto) { return false; });
        CHECK_FALSE(checker.check(7));
    }
    SUBCASE("test") {
        checker.setTest([](auto) { return false; });
        CHECK_FALSE(checker.check(7));
    }
}

TEST_CASE("test-reset") {
    Checker checker;
    SUBCASE("global") {
        checker.setGlobal([](auto) { return false; });
        checker.resetGlobal();
        CHECK(checker.check(7));
    }
    SUBCASE("suite") {
        checker.setSuite([](auto) { return false; });
        checker.resetSuite();
        CHECK(checker.check(7));
    }
    SUBCASE("test") {
        checker.setTest([](auto) { return false; });
        checker.resetTest();
        CHECK(checker.check(7));
    }
}

class TestcaseGen {
public:
    int gen;
};

static_assert(!has_gen_v<int>);
static_assert(!has_gen_v<DummyTestcase>);
static_assert(has_gen_v<TestcaseGen>);
