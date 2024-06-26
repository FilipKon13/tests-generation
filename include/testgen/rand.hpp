#ifndef TESTGEN_RAND_HPP_
#define TESTGEN_RAND_HPP_

#include "util.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <numeric>
#include <type_traits>

constexpr uint64_t TESTGEN_SEED = 0;

namespace test {

class Xoshiro256pp {
    // Suppress magic number linter errors (a lot of that in here and that is normal for a RNG)
public:
    using result_type = uint64_t;

private:
    std::array<result_type, 4> s{};

    static inline result_type rotl(result_type x, unsigned k) {
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
        return (x << k) | (x >> (64U - k));
    }

    void advance() {
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
        auto const t = s[1] << 17U;
        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];
        s[2] ^= t;
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
        s[3] = rotl(s[3], 45U);
    }

    void jump() {
        static constexpr std::array JUMP = {0x180ec6d33cfd0abaULL, 0xd5a61266f0c9392cULL, 0xa9582618e03fc9aaULL, 0x39abdc4529b1661cULL};
        static constexpr unsigned RESULT_TYPE_WIDTH = 64;
        std::array<result_type, 4> t{};
        for(auto jump : JUMP) {
            for(unsigned b = 0; b < RESULT_TYPE_WIDTH; b++) {
                if((jump & UINT64_C(1) << b) != 0) {
                    t[0] ^= s[0];
                    t[1] ^= s[1];
                    t[2] ^= s[2];
                    t[3] ^= s[3];
                }
                advance();
            }
        }
        std::copy(t.begin(), t.end(), s.begin());
    }

public:
    explicit Xoshiro256pp(result_type seed) noexcept {
        auto next_seed = [x = seed]() mutable {
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
            auto z = (x += 0x9e3779b97f4a7c15ULL);
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
            z = (z ^ (z >> 30U)) * 0xbf58476d1ce4e5b9ULL;
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
            z = (z ^ (z >> 27U)) * 0x94d049bb133111ebULL;
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
            return z ^ (z >> 31U);
        };
        for(auto & v : s) {
            v = next_seed();
        }
    }

    [[nodiscard]] result_type operator()() noexcept {
        auto const result = rotl(s[0] + s[3], 23) + s[0];
        advance();
        return result;
    }

    [[nodiscard]] Xoshiro256pp fork() noexcept {
        auto const result = *this;
        jump();
        return result;
    }

    static constexpr result_type max() {
        return UINT64_MAX;
    }

    static constexpr result_type min() {
        return 0;
    }
};

using gen_type = Xoshiro256pp;

template<typename T>
class GeneratorWrapper {
    T * gen{};

public:
    explicit GeneratorWrapper(T & gen) noexcept :
      gen(&gen) {}
    GeneratorWrapper & operator=(T & gen) {
        this->gen = &gen;
        return *this;
    }
    ~GeneratorWrapper() noexcept = default;
    GeneratorWrapper(GeneratorWrapper const &) noexcept = default;
    GeneratorWrapper(GeneratorWrapper &&) noexcept = default;
    GeneratorWrapper & operator=(GeneratorWrapper const &) noexcept = default;
    GeneratorWrapper & operator=(GeneratorWrapper &&) noexcept = default;

    static constexpr auto max() {
        return T::max();
    }

    static constexpr auto min() {
        return T::min();
    }

    operator T &() { //NOLINT allow nonexplicit use as reference to generator inside
        return *gen;
    }

    typename T::result_type operator()() noexcept {
        return (*gen)();
    }

    T & generator() {
        return *gen;
    }
};

template<typename T>
class Generating {
public:
    virtual T generate(gen_type & gen) const = 0;
    virtual ~Generating() noexcept = default;
};

template<typename T>
struct is_generating { //NOLINT(readability-identifier-naming)
private:
    template<typename V>
    static decltype(static_cast<const Generating<V> &>(std::declval<T>()), std::true_type{})
    helper(const Generating<V> &);
    static std::false_type helper(...); /* fallback */
public:
    //NOLINTNEXTLINE readability-identifier-naming and c-vararg
    static constexpr bool value = decltype(helper(std::declval<T>()))::value;
};

template<typename T>
inline constexpr bool is_generating_v = is_generating<T>::value; //NOLINT(readability-identifier-naming)

template<typename T>
struct uni_dist {
private:
    static_assert(std::is_integral_v<T>);
    T begin, end;

public:
    uni_dist(T begin, T end) :
      begin(begin), end(end) {
        assume(begin <= end);
    }

    template<typename Gen>
    T operator()(Gen && gen) const {
        return uni_dist::gen(begin, end, std::forward<Gen>(gen));
    }

    template<typename Gen>
    static T gen(T begin, T end, Gen && gen) {
        auto const range = end - begin + 1;
        return (gen() % range) + begin; // not really uniform, but close enough
    }
};

namespace detail {

template<typename Iter>
void iter_swap(Iter a, Iter b) {
    std::swap(*a, *b);
}
} /* namespace detail */

template<typename Iter, typename Gen>
void shuffle_sequence(Iter begin, Iter end, Gen && gen) {
    if(begin == end) {
        return;
    }
    for(auto it = begin; ++it != end;) {
        detail::iter_swap(it, begin + uni_dist<size_t>(0, std::distance(begin, it))(gen));
    }
}

std::vector<uint> get_permutation(int n, gen_type & gen) {
    std::vector<uint> V(n);
    std::iota(std::begin(V), std::end(V), 0U);
    shuffle_sequence(std::begin(V), std::end(V), gen);
    return V;
}

/* CRTP, assumes Derived has 'generator()' method/field */
template<typename Derived>
class RngUtilities {
    decltype(auto) gen() {
        return static_cast<Derived *>(this)->generator();
    }

public:
    // shuffle RA sequence
    template<typename Iter>
    void shuffle(Iter b, Iter e) {
        shuffle_sequence(b, e, gen());
    }

    // shuffle RA sequence
    template<typename Container>
    void shuffle(Container & cont) {
        shuffle(std::begin(cont), std::end(cont));
    }

    // get random int32 in [from:to], inclusive
    int32_t randInt(int32_t from, int32_t to) {
        return uni_dist<int32_t>::gen(from, to, gen());
    }

    // get random int64 in [from:to], inclusive
    int64_t randLong(int64_t from, int64_t to) {
        return uni_dist<int64_t>::gen(from, to, gen());
    }
};

} /* namespace test */

#endif /* TESTGEN_RAND_HPP_ */