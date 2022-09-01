#ifndef TESTGEN_SEQUENCE_HPP_
#define TESTGEN_SEQUENCE_HPP_

#include "rand.hpp"

#include <algorithm>
#include <ostream>
#include <type_traits>
#include <vector>

namespace test {

template<typename T>
class Sequence : public std::vector<T> {

    template<typename Gen, std::enable_if_t<std::is_invocable_v<Gen>, int> = 0>
    static void seqGenerate(Sequence & s, Gen&& gen) {
        std::generate(s.begin(), s.end(), gen);
    }

    template<typename Gen, std::enable_if_t<std::is_invocable_v<Gen, unsigned>, int> = 0>
    static void seqGenerate(Sequence & s, Gen&& gen) {
        std::generate(s.begin(), s.end(), [&gen, cnt = 0U] () mutable {return std::invoke(gen, cnt++);});
    }

public:
    using std::vector<T>::vector;

    template<typename Gen, typename = std::enable_if_t<std::is_invocable_v<Gen> || std::is_invocable_v<Gen, unsigned>>>
    Sequence(std::size_t size, Gen&& gen) : std::vector<T>(size) {
        seqGenerate(*this, std::forward<Gen>(gen));
    }

    Sequence operator+(Sequence const & x) const {
        Sequence res(this->size() + x.size());
        auto const it = std::copy(this->begin(), this->end(), res.begin());
        std::copy(x.begin(), x.end(), it);
        return res;
    }

    Sequence& operator+=(Sequence const & x) {
        auto const old_size = this->size();
        this->resize(old_size + x.size());
        std::copy(x.begin(), x.end(), this->begin() + old_size);
        return * this;
    }

    friend std::ostream& operator<<(std::ostream & s, Sequence const & x) {
        auto it = x.begin();
        auto const end = x.end();
        if(it == end)   {return s;}
        s << *it++;
        while(it != end) {s << ' ' << *it++;}
        return s;
    }
};

} /* namespace test */


#endif /* TESTGEN_SEQUENCE_HPP_ */