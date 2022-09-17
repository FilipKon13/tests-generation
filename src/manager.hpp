#ifndef TESTGEN_MANAGER_HPP_
#define TESTGEN_MANAGER_HPP_

#include "testing.hpp"
#include "rand.hpp"

#include <algorithm>
#include <fstream>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <variant>


namespace test {

enum TestType : int8_t { OCEN };

namespace detail {
struct index {
    unsigned test;                          //NOLINT(misc-non-private-member-variables-in-classes)
    std::variant<TestType, unsigned> suite; //NOLINT(misc-non-private-member-variables-in-classes)

    [[nodiscard]] constexpr bool operator==(index const & x) const {
        return test == x.test && suite == x.suite;
    }

    class hash {
        constexpr inline static unsigned SHIFT = 10U;
    public:
        [[nodiscard]] constexpr std::size_t operator()(index const & indx) const {
            return static_cast<size_t>((indx.test << SHIFT) ^ 
            std::visit(combine{
                []([[maybe_unused]] TestType x){return 0U;},
                [](unsigned x){return x+1;}
            }, indx.suite));
        }
    };
};
} /* namespace detail */

template<typename StreamType = std::ofstream>
class OIOIOIManager {

    bool change_if_taken() {
        if(auto const it = cases.find(curr_index); it != cases.end()) {
            curr_test = &it->second;
            return true;
        }
        return false;
    }

    void change_to_new_stream(std::string const & name) {
        auto const it = cases.try_emplace(curr_index,
            std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(std::hash<std::string>{}(name)));
        curr_test = &it.first->second;
    }

    class test_info : private std::pair<StreamType, gen_type> {
    public:
        using std::pair<StreamType, gen_type>::pair;
        [[nodiscard]] constexpr StreamType& stream() noexcept {
            return this->first;
        }
        [[nodiscard]] constexpr gen_type& generator() noexcept {
            return this->second;
        }
    };
    using index = detail::index;
    test_info * curr_test{nullptr};
    index curr_index{};
    std::string abbr;
    std::unordered_map<index, test_info, index::hash> cases{};

public:

    explicit OIOIOIManager(std::string abbr, bool ocen = false) : abbr(std::move(abbr)) {
        if(ocen)    {test(1, OCEN);} // pro1ocen.in
        else        {test(1, 1);}    // pro1a.in
    }

    OIOIOIManager() = delete;
    OIOIOIManager(OIOIOIManager const &) = delete;
    OIOIOIManager(OIOIOIManager&&) noexcept = default;
    ~OIOIOIManager() = default;
    OIOIOIManager& operator=(OIOIOIManager const &) = delete;
    OIOIOIManager& operator=(OIOIOIManager&&) noexcept = default;

    [[nodiscard]] constexpr StreamType & stream() const noexcept {
        return curr_test->stream();
    }

    [[nodiscard]] constexpr gen_type & generator() const noexcept {
        return curr_test->generator();
    }

    void nextTest() {
        std::visit(combine{
            [this](TestType x)  {this->test(this->curr_index.test + 1, x);},
            [this](unsigned x)       {this->test(this->curr_index.test + 1, x);}
        },curr_index.suite);
    }

    void nextSuite() {
        std::visit(combine{
            [this]([[maybe_unused]] TestType x) {this->test(1, 1);},
            [this](unsigned x)      {this->test(1, x + 1);}
        },curr_index.suite);
    }

    void test(unsigned test, unsigned suite) {
        curr_index = {test, suite};
        if(change_if_taken())   {return;}
        std::string suffix{};
        auto nr = test - 1;
        constexpr unsigned size = 'z' - 'a' + 1;
        do {
            suffix += 'a' + (nr % size);
            nr /= size;
        } while(nr != 0);
        std::reverse(std::begin(suffix), std::end(suffix));
        change_to_new_stream(abbr + std::to_string(suite) + suffix + ".in");
    }

    void test(unsigned test, TestType ocen) {
        curr_index = {test, ocen};
        if(change_if_taken()) {return;}
        change_to_new_stream(abbr + std::to_string(test) + "ocen.in");
    }
    
};

} /* namespace test */


#endif /* TESTGEN_MANAGER_HPP_ */