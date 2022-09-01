#ifndef TESTGEN_RAND_HPP_
#define TESTGEN_RAND_HPP_

#include <random>

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


template<typename T, typename enable = void> class uniform_distribution;

template<typename T>
class uniform_distribution<T, std::enable_if_t<std::is_integral_v<T>>> {
    std::uniform_int_distribution<T> dist;
public:
    uniform_distribution(T begin, T end) : dist{begin, end} {}
    T operator()(gen_type & gen) {
        return dist(gen);
    }
};

template<typename T>
class uniform_distribution<T, std::enable_if_t<std::is_floating_point_v<T>>> {

};

template<typename... T>
struct combine : T... {using T::operator()...;};        //NOLINT(fuchsia-trailing-return)

template<typename... T> combine(T...) -> combine<T...>; //NOLINT(fuchsia-trailing-return) 

} /* namespace test */


#endif /* TESTGEN_RAND_HPP_ */