#include "doctest.h"
#include <testgen/rand.hpp>
#include <algorithm>
using namespace test;

using namespace std;

TEST_CASE("test_standard") {
    UniDist<int> dist(1, 10);
    vector<int> V(1000);
    std::generate(begin(V), end(V), [gen = gen_type{12}, &dist]() mutable { return dist(gen); });
    for(auto v : V) {
        CHECK_UNARY(1 <= v && v <= 10);
    }
    for(int i = 1; i <= 10; i++) {
        CHECK(find(begin(V), end(V), i) != end(V));
    }
}

TEST_CASE("test_single") {
    UniDist<int> dist(7, 7);
    gen_type gen{13};
    for(int i = 1; i <= 1000; i++) {
        CHECK(dist(gen) == 7);
    }
}
