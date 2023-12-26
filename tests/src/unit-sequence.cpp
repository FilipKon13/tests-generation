#include "doctest.h"
#include <testgen/sequence.hpp>
#include <sstream>
using namespace test;
using namespace std;

TEST_CASE("test_construction") {
    Sequence<int> const s(3, 5);

    CHECK(s.size() == size_t{3});
    CHECK(s[0] == 5);
    CHECK(s[1] == 5);
    CHECK(s[2] == 5);
}

TEST_CASE("test_add") {
    Sequence<int> s1(2, 2);
    Sequence<int> s2({1, 2, 3});

    auto const s = s1 + s2;
    s1 += s2;

    CHECK(s.size() == size_t{5});
    CHECK(s[0] == 2);
    CHECK(s[2] == 1);
    CHECK(s == s1);
}

TEST_CASE("test_print") {
    stringstream s{};
    s << Sequence<int>({1, 2, 3, 4, 5});

    CHECK(s.str() == "1 2 3 4 5");
}

TEST_CASE("test_generation_no_indx") {
    Sequence<int> const s(4, [i = 0]() mutable -> int { return ++i; });
    Sequence<int> const exp({1, 2, 3, 4});

    CHECK(s == exp);
}

TEST_CASE("test_generation_indx") {
    Sequence<int> const s(4, [](int x) { return x; });
    Sequence<int> const exp({0, 1, 2, 3});

    CHECK(s == exp);
}

TEST_CASE("test_uni_sequence") {
    UniSequence<int> const seq(3, 1, 1);
    gen_type gen{0};

    CHECK(seq.generate(gen) == Sequence<int>({1, 1, 1}));
}

TEST_CASE("test_finite_sequence") {
    gen_type gen{0};
    SUBCASE("in-place c-style array construction") {
        using arr = int[3];
        FiniteSequence seq(20, arr{10, 20, 30});
        auto const res = seq.generate(gen);
        CHECK(res.size() == size_t{20});
        for(auto v : res)
            CHECK_UNARY(v == 10 || v == 20 || v == 30);
        for(int i = 1; i <= 3; ++i)
            CHECK(find(res.begin(), res.end(), i * 10) != res.end());
    }
    SUBCASE("initializer list construction") {
        FiniteSequence seq(20, {10, 20, 30});
        auto const res = seq.generate(gen);
        CHECK(res.size() == size_t{20});
        for(auto v : res)
            CHECK_UNARY(v == 10 || v == 20 || v == 30);
        for(int i = 1; i <= 3; ++i)
            CHECK(find(res.begin(), res.end(), i * 10) != res.end());
    }
    SUBCASE("c-style array construction") {
        int arr[] = {10, 20, 30};
        FiniteSequence seq(20, arr);
        auto const res = seq.generate(gen);
        CHECK(res.size() == size_t{20});
        for(auto v : res)
            CHECK_UNARY(v == 10 || v == 20 || v == 30);
        for(int i = 1; i <= 3; ++i)
            CHECK(find(res.begin(), res.end(), i * 10) != res.end());
    }
    SUBCASE("deducing type") {
        auto res = FiniteSequence(5, {1.0, 2.0, 3.0}).generate(gen).front();
        static_assert(std::is_same_v<decltype(res), double>);
    }
}