#ifndef TESTGEN_RAND_HPP_
#define TESTGEN_RAND_HPP_

#include <cassert>
#include <memory>
#include <random>
#include <type_traits>

namespace test {

using gen_type = std::mt19937;

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