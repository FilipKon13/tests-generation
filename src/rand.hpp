#ifndef TESTGEN_RAND_HPP_
#define TESTGEN_RAND_HPP_

#include <random>

namespace test {

using gen_type = std::mt19937;

template<typename T>
struct Generating {
    virtual T generate(gen_type & gen) const = 0; // NOLINT(google-runtime-references) this is how random works
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
    T operator()(gen_type & gen) { // NOLINT(google-runtime-references) this is how random works
        return dist(gen);
    }
};

template<typename T>
class uniform_distribution<T, std::enable_if_t<std::is_floating_point_v<T>>> {

};

} /* namespace test */


#endif /* TESTGEN_RAND_HPP_ */