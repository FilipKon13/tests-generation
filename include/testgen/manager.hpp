#ifndef TESTGEN_MANAGER_HPP_
#define TESTGEN_MANAGER_HPP_

#include "rand.hpp"
#include "testing.hpp"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace test {

enum TestType : uint8_t { OCEN = UINT8_MAX };

namespace detail {
struct index {
    unsigned test;
    std::variant<TestType, unsigned> suite;

    [[nodiscard]] constexpr bool operator==(index const & x) const {
        return test == x.test && suite == x.suite;
    }

    struct hash {
        [[nodiscard]] constexpr std::size_t operator()(index const & indx) const {
            constexpr auto SHIFT = 10U;
            return static_cast<size_t>(
                (indx.test << SHIFT) ^ std::visit([](auto x) { return x + 1U; }, indx.suite));
        }
    };
};
} /* namespace detail */

template<typename StreamType = std::ofstream>
class OIOIOIManager {
    bool changeIfTaken() {
        if(auto const it = cases.find(curr_index); it != cases.end()) {
            curr_test = &it->second;
            return true;
        }
        return false;
    }

    void changeToNewStream(std::string const & name) {
        auto const it = cases.try_emplace(curr_index,
                                          std::piecewise_construct,
                                          std::forward_as_tuple(name),
                                          std::forward_as_tuple(source_generator.fork()));
        curr_test = &it.first->second;
    }

    struct test_info : private std::pair<StreamType, gen_type> {
        using std::pair<StreamType, gen_type>::pair;
        [[nodiscard]] constexpr StreamType & stream() noexcept {
            return this->first;
        }
        [[nodiscard]] constexpr gen_type & generator() noexcept {
            return this->second;
        }
    };
    using index = detail::index;
    test_info * curr_test{nullptr};
    index curr_index;
    std::string abbr;
    std::unordered_map<index, test_info, index::hash> cases{};
    gen_type source_generator{TESTGEN_SEED};

public:
    // CUSTOM: change default first suite to 0U if used for example tests
    explicit OIOIOIManager(std::string abbr, bool ocen = false) :
      curr_index{0, ocen ? std::variant<TestType, unsigned>(OCEN) : std::variant<TestType, unsigned>(/* CUSTOM */ 1U)}, abbr(std::move(abbr)) {}

    OIOIOIManager() = delete;
    OIOIOIManager(OIOIOIManager const &) = delete;
    OIOIOIManager(OIOIOIManager &&) noexcept = default;
    ~OIOIOIManager() = default;
    OIOIOIManager & operator=(OIOIOIManager const &) = delete;
    OIOIOIManager & operator=(OIOIOIManager &&) noexcept = default;

    void setMainSeed(gen_type::result_type seed) {
        source_generator = gen_type(seed);
    }

    [[nodiscard]] constexpr StreamType & stream() const noexcept {
        return curr_test->stream();
    }

    [[nodiscard]] constexpr gen_type & generator() const noexcept {
        return curr_test->generator();
    }

    void nextTest() {
        std::visit([this](auto x) { this->test(this->curr_index.test + 1, x); },
                   curr_index.suite);
    }

    void nextSuite() {
        std::visit(combine{
                       [this]([[maybe_unused]] TestType) {
                           this->test(1, 1);
                       },
                       [this](unsigned x) {
                           this->test(1, x + 1);
                       }},
                   curr_index.suite);
    }

    void test(unsigned test, unsigned suite) {
        curr_index = {test, suite};
        if(changeIfTaken()) { return; }
        changeToNewStream(getFilename());
    }

    void test(unsigned test, TestType ocen) {
        curr_index = {test, ocen};
        if(changeIfTaken()) { return; }
        changeToNewStream(getFilename());
    }

    [[nodiscard]] std::string getFilename() const {
        if(unsigned const * suite = std::get_if<unsigned>(&curr_index.suite)) {
            std::string suffix{};
            auto nr = curr_index.test - 1;
            constexpr unsigned SIZE = 'z' - 'a' + 1;
            do {
                suffix += 'a' + (nr % SIZE);
                nr /= SIZE;
            } while(nr != 0);
            std::reverse(std::begin(suffix), std::end(suffix));
            return abbr + std::to_string(*suite) + suffix + ".in";
        }
        return abbr + std::to_string(curr_index.test) + "ocen.in";
    }
};

} /* namespace test */

#endif /* TESTGEN_MANAGER_HPP_ */