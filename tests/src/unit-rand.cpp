#include <doctest.h>

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

TEST_CASE("test-rng-utilities-custom") {
    struct rng : RngUtilities<rng> {
        gen_type g{15};
        gen_type & generator() {
            return g;
        }
    } rng;
    std::array V = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto copy = V;
    rng.shuffle(std::begin(V), std::end(V));
    CHECK_NE(V, copy); // should pass 1/10!
    std::sort(std::begin(V), std::end(V));
    CHECK_EQ(V, copy);
}

TEST_CASE("test-rng-utilities-wrapper") {
    gen_type g{17};
    struct rng : public GeneratorWrapper<gen_type>, public RngUtilities<rng> {
        using GeneratorWrapper::GeneratorWrapper;
    } rng{g};
    std::array V = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto copy = V;
    rng.shuffle(std::begin(V), std::end(V));
    CHECK_NE(V, copy); // should pass 1/10!
    std::sort(std::begin(V), std::end(V));
    CHECK_EQ(V, copy);
}

TEST_CASE("test-generator-wrapper-algo") {
    gen_type g{26};
    GeneratorWrapper<gen_type> wrapper{g};
    std::array V = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto copy = V;
    shuffle_sequence(std::begin(V), std::end(V), wrapper);
    CHECK_NE(V, copy); // should pass 1/10!
    std::sort(std::begin(V), std::end(V));
    CHECK_EQ(V, copy);
}

TEST_CASE("test-generator-wrapper-deduction") {
    gen_type const g{26};
    static_assert(std::is_same_v<decltype(GeneratorWrapper{g}), GeneratorWrapper<const gen_type>>);
}

TEST_CASE("test-static-functions") {
    static_assert(gen_type::min() == GeneratorWrapper<gen_type>::min());
    static_assert(gen_type::max() == GeneratorWrapper<gen_type>::max());
}

TEST_CASE("test-passing-generator") {
    struct test_gen : RngUtilities<test_gen> {
        test_gen() = default;
        test_gen(test_gen const &) = delete;
        test_gen & operator=(test_gen const &) = delete;
        test_gen(test_gen &&) = delete;
        test_gen & operator=(test_gen &&) = delete;
        ~test_gen() = default;
        auto operator()() {
            return 0;
        }
        test_gen & generator() {
            return *this;
        }
    } utils;
    utils.randInt(0, 0);
    utils.randLong(0, 0);
    std::array a = {0, 1, 2};
    utils.shuffle(std::begin(a), std::end(a));
}