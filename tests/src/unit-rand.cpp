#include "doctest.h"
#include <type_traits>

#include <testgen/rand.hpp>
using namespace test;

static_assert(std::is_copy_assignable_v<GeneratorWrapper<gen_type>>);
static_assert(std::is_copy_constructible_v<GeneratorWrapper<gen_type>>);
static_assert(std::is_move_assignable_v<GeneratorWrapper<gen_type>>);
static_assert(std::is_move_constructible_v<GeneratorWrapper<gen_type>>);

TEST_CASE("test-conversion-generator-wrapper") {
    gen_type gen{12};
    GeneratorWrapper<gen_type> wrapped{gen};
    auto const fun = [](gen_type & generator) {
        return generator();
    };
    fun(wrapped);
}

TEST_CASE("test-use-wrapper") {
    gen_type g1{7};
    gen_type g2{7};
    GeneratorWrapper<gen_type> wrapped{g1};
    CHECK_EQ(g2(), g1());
    CHECK_EQ(g2(), wrapped());
    CHECK_EQ(g2(), g1());
    CHECK_EQ(g2(), wrapped());
}