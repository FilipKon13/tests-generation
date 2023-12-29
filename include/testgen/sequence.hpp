#ifndef TESTGEN_SEQUENCE_HPP_
#define TESTGEN_SEQUENCE_HPP_

#include "rand.hpp"

#include <algorithm>
#include <functional>
#include <initializer_list>
#include <ostream>
#include <type_traits>
#include <vector>

namespace test {

template<typename T>
class Sequence : public std::vector<T> {
    template<typename Gen, std::enable_if_t<std::is_invocable_v<Gen>, int> = 0>
    static void seqGenerate(Sequence & s, Gen && gen) {
        std::generate(s.begin(), s.end(), gen);
    }

    template<typename Gen, std::enable_if_t<std::is_invocable_v<Gen, unsigned>, int> = 0>
    static void seqGenerate(Sequence & s, Gen && gen) {
        std::generate(s.begin(), s.end(), [&gen, cnt = 0U]() mutable { return std::invoke(gen, cnt++); });
    }

public:
    using std::vector<T>::vector;

    template<typename Gen, typename = std::enable_if_t<std::is_invocable_v<Gen> || std::is_invocable_v<Gen, unsigned>>>
    Sequence(std::size_t size, Gen && gen) :
      std::vector<T>(size) {
        seqGenerate(*this, std::forward<Gen>(gen));
    }

    Sequence operator+(Sequence const & x) const {
        Sequence res(this->size() + x.size());
        auto const it = std::copy(this->begin(), this->end(), res.begin());
        std::copy(x.begin(), x.end(), it);
        return res;
    }

    Sequence & operator+=(Sequence const & x) {
        auto const old_size = this->size();
        this->resize(old_size + x.size());
        std::copy(x.begin(), x.end(), this->begin() + old_size);
        return *this;
    }

    friend std::ostream & operator<<(std::ostream & s, Sequence const & x) {
        auto it = x.begin();
        auto const end = x.end();
        if(it == end) { return s; }
        s << *it++;
        while(it != end) {
            s << ' ' << *it++;
        }
        return s;
    }
};

template<typename Dist, typename T>
class DistSequence : Generating<Sequence<T>> {
    std::size_t n;
    Dist dist;

public:
    constexpr DistSequence(std::size_t n, T begin, T end) :
      n(n), dist(begin, end) {}
    Sequence<T> generate(gen_type & gen) const override {
        return Sequence<T>(n, [&] { return dist(gen); });
    }
};

template<typename T>
using UniSequence = DistSequence<uni_dist<T>, T>;

template<typename T, std::size_t S>
class FiniteSequence : Generating<Sequence<T>> {
    std::size_t n;
    std::array<T, S> elems;

    template<std::size_t... Indx>
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
    static std::array<T, S> construct(T const (&arr)[S], std::index_sequence<Indx...> /* unused */) {
        return std::array<T, S>({arr[Indx]...});
    }

public:
    constexpr FiniteSequence(std::size_t n, std::array<T, S> const & arr) :
      n(n), elems(arr) {}

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
    constexpr FiniteSequence(std::size_t n, T const (&arr)[S]) :
      n(n), elems(construct(arr, std::make_index_sequence<S>{})) {}
    Sequence<T> generate(gen_type & gen) const override {
        return Sequence<T>(n, [&, dist = uni_dist<std::size_t>(0, S - 1)] { return elems.at(dist(gen)); });
    }
};

} /* namespace test */

#endif /* TESTGEN_SEQUENCE_HPP_ */