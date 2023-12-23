#ifndef TESTGEN_RAND_HPP_
#define TESTGEN_RAND_HPP_

#include <algorithm>
#include <memory>
#include <random>
#include <type_traits>

namespace test {

class xoshiro256pp {
    static inline uint64_t rotl(uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }
    uint64_t s[4];

    void advance() {
        auto const t = s[1] << 17;
        s[2] ^= s[0]; s[3] ^= s[1]; s[1] ^= s[2]; s[0] ^= s[3];
        s[2] ^= t;
        s[3] = rotl(s[3], 45);
    }

    void jump() {
        static constexpr uint64_t JUMP[] =
            { 0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c };
        uint64_t t[4]{};
        for(int i = 0; i < 4; i++)
            for(int b = 0; b < 64; b++) {
                if (JUMP[i] & UINT64_C(1) << b) {
                    t[0] ^= s[0];
                    t[1] ^= s[1];
                    t[2] ^= s[2];
                    t[3] ^= s[3];
                }
                advance();
            }
        std::copy(t,t+4,s);
    }

    static inline uint64_t next_seed(uint64_t & x) {
        auto z = (x += 0x9e3779b97f4a7c15);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
        z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
        return z ^ (z >> 31);
    }

public:
    explicit xoshiro256pp(uint64_t seed) noexcept {
        for(int i = 0; i < 4; i++) {
            s[i] = next_seed(seed);
        }
    }

    [[nodiscard]] uint64_t operator()() noexcept {
        auto const result = rotl(s[0] + s[3], 23) + s[0];
        advance();
        return result;
    }

    [[nodiscard]] xoshiro256pp fork() noexcept {
        auto const result = *this;
        jump();
        return result; // NRVO still applies here
    }
};

using gen_type = xoshiro256pp;

template<typename T>
struct Generating {
    virtual T generate(gen_type & gen) const = 0;
};

template<typename T>
struct is_generating
{
private:
    template<typename V>
    static decltype(static_cast<const Generating<V>&>(std::declval<T>()), std::true_type{})
    helper(const Generating<V>&);
    
    static std::false_type helper(...); /* fallback */
public:
    static constexpr bool value = decltype(helper(std::declval<T>()))::value;
};

template<typename T>
class UniDist {
    T _begin, _end;
public:
    UniDist(T begin, T end) : _begin(begin), _end(end) {
        assert(begin <= end);
    }
    template<typename Gen>
    T operator()(Gen&& gen) const {
        return UniDist::gen(_begin, _end, std::forward<Gen>(gen));
    }
    template<typename Gen>
    static T gen(T begin, T end, Gen&& gen) {
        if constexpr (std::is_integral_v<T>) { // make this better
            auto const range = end - begin + 1;
            auto res = gen() % range;
            return res + begin;
        } else {
            typedef typename std::remove_reference<Gen>::type gen_t;
            auto const urange = gen_t::max() - gen_t::min();
            auto const range = end - begin;
            return (static_cast<T>(gen()) / urange) * range + begin;
        }
    }
};


template<typename T>
class uniform_real_distribution {

};

template<typename... T> struct combine : T... {using T::operator()...;};

template<typename... T> combine(T...) -> combine<T...>; 


namespace detail {
    template<typename T, typename Gen>
    std::pair<T,T> generate_two(T x, T y, Gen&& gen) {
        T v = UniDist<T>::gen(x,y,gen);
        return {v/x, v%x};
    }
} /* namespace detail */

template<typename Iter, typename Gen>
void random_permute(Iter begin, Iter end, Gen&& gen) {
    if(begin == end) {
        return;
    }
    auto const len = end - begin;
    auto const range = Gen::max() - Gen::min();
    if(range / len >= len) { // faster variant
        auto it = begin + 1;
        if(len % 2 == 0) {
            swap(it++, begin + UniDist<decltype(len)>::gen(0, 1, gen));
        }
        while(it != end) {
            auto cnt = it - begin;
            auto p = detail::generate_two(cnt, cnt+1, gen);
            swap(it++, begin + p.first);
            swap(it++, begin + p.second);
        }
    } else { // for really big ranges
        for(auto it = begin; ++it != end;) {
            swap(it, begin + UniDist<decltype(len)>::gen(0, it - begin, gen));
        }
    }
}

} /* namespace test */


#endif /* TESTGEN_RAND_HPP_ */