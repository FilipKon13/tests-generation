#include <doctest.h>

#include <sstream>

#include <testgen/sequence.hpp>
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
    Sequence<int> const s2({1, 2, 3});

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