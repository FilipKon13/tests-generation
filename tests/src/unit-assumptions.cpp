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

TEST_CASE("test-utility-deduction-functor") {
    class MoveOnlyFunctor {
    public:
        MoveOnlyFunctor() = default;
        MoveOnlyFunctor(MoveOnlyFunctor const &) = delete;
        MoveOnlyFunctor(MoveOnlyFunctor &&) = default;
        MoveOnlyFunctor & operator=(MoveOnlyFunctor const &) = delete;
        MoveOnlyFunctor & operator=(MoveOnlyFunctor &&) = default;
        ~MoveOnlyFunctor() = default;
        bool operator()(const int & /* unused */) {
            return true;
        }
    };

    class CopyOnlyFunctor {
    public:
        CopyOnlyFunctor() = default;
        CopyOnlyFunctor(CopyOnlyFunctor const &) = default;
        CopyOnlyFunctor(CopyOnlyFunctor &&) = delete;
        CopyOnlyFunctor & operator=(CopyOnlyFunctor const &) = delete;
        CopyOnlyFunctor & operator=(CopyOnlyFunctor &&) = default;
        ~CopyOnlyFunctor() = default;
        bool operator()(const int & /* unused */) {
            return true;
        }
    };
    SUBCASE("cc") {
        const CopyOnlyFunctor c1;
        const CopyOnlyFunctor c2;
        auto conj = c1 && c2;
        CHECK_UNARY(conj(1));
    }
    SUBCASE("cm") {
        const CopyOnlyFunctor c;
        MoveOnlyFunctor m;
        auto conj = c && std::move(m);
        CHECK_UNARY(conj(1));
    }
    SUBCASE("mc") {
        MoveOnlyFunctor m;
        const CopyOnlyFunctor c;
        auto conj = std::move(m) && c;
        CHECK_UNARY(conj(1));
    }
    SUBCASE("mm") {
        MoveOnlyFunctor m1;
        MoveOnlyFunctor m2;
        auto conj = std::move(m1) && std::move(m2);
        CHECK_UNARY(conj(1));
    }
    SUBCASE("inplace") {
        auto conj = MoveOnlyFunctor() && MoveOnlyFunctor();
        CHECK_UNARY(conj(1));
    }
}

TEST_CASE("test-utility-deduction-lambda") {
    SUBCASE("cc") {
        auto l1 = [](auto) {
            return true;
        };
        auto l2 = [](auto) {
            return true;
        };
        auto conj = l1 && l2;
        CHECK_UNARY(conj(1));
    }
    SUBCASE("cm") {
        auto l1 = [](auto) {
            return true;
        };
        auto l2 = [](auto) {
            return true;
        };
        auto conj = l1 && std::move(l2);
        CHECK_UNARY(conj(1));
    }
    SUBCASE("mc") {
        auto l1 = [](auto) {
            return true;
        };
        auto l2 = [](auto) {
            return true;
        };
        auto conj = std::move(l1) && l2;
        CHECK_UNARY(conj(1));
    }
    SUBCASE("mm") {
        auto l1 = [](auto) {
            return true;
        };
        auto l2 = [](auto) {
            return true;
        };
        auto conj = std::move(l1) && std::move(l2);
        CHECK_UNARY(conj(1));
    }
    SUBCASE("inplace") {
        auto conj = [](auto) {
            return true;
        } && [](auto) {
            return true;
        };
        CHECK_UNARY(conj(1));
    }
}

TEST_CASE("test-utility-logic") {
    auto true_l = [](auto) {
        return true;
    };
    auto false_l = [](auto) {
        return false;
    };
    CHECK_UNARY((true_l && true_l)(1));
    CHECK_UNARY_FALSE((false_l && true_l)(1));
    CHECK_UNARY_FALSE((true_l && false_l)(1));
    CHECK_UNARY_FALSE((false_l && false_l)(1));
    CHECK_UNARY((true_l || true_l)(1));
    CHECK_UNARY((false_l || true_l)(1));
    CHECK_UNARY((true_l || false_l)(1));
    CHECK_UNARY_FALSE((false_l || false_l)(1));
}
