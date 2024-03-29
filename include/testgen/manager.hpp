#ifndef TESTGEN_MANAGER_HPP_
#define TESTGEN_MANAGER_HPP_

#include "rand.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace test {

namespace detail {
struct index {
    unsigned test;
    unsigned suite;

    [[nodiscard]] constexpr bool operator==(index const & x) const {
        return test == x.test && suite == x.suite;
    }

    struct hash {
        [[nodiscard]] constexpr std::size_t operator()(index const & indx) const {
            constexpr auto SHIFT = 10U;
            return (indx.test << SHIFT) ^ indx.suite;
        }
    };
};
} /* namespace detail */

enum Verbocity {
    SILENT = 0,
    VERBOSE = 1
};

template<Verbocity Verbose = VERBOSE, typename StreamType = std::ofstream>
class OIOIOIManager {
    bool changeIfTaken() {
        if(auto const it = cases.find(curr_index); it != cases.end()) {
            curr_test = &it->second;
            return true;
        }
        return false;
    }

    void changeToNewStream(std::string const & name) {
        if constexpr(Verbose == Verbocity::VERBOSE) {
            std::cout << "Printing to: " << name << '\n';
        }
        auto const it = cases.try_emplace(curr_index,
                                          std::piecewise_construct,
                                          std::forward_as_tuple(name),
                                          std::forward_as_tuple(source_generator.fork()));
        curr_test = &it.first->second;
    }

    void clearStream() {
        curr_test = nullptr;
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
    gen_type source_generator;

public:
    explicit OIOIOIManager(std::string abbr, bool ocen = true, uint64_t seed = TESTGEN_SEED) :
      curr_index{0U, ocen ? 0U : 1U}, abbr{std::move(abbr)}, source_generator{seed} {}

    OIOIOIManager() = delete;
    OIOIOIManager(OIOIOIManager const &) = delete;
    OIOIOIManager(OIOIOIManager &&) noexcept = default;
    ~OIOIOIManager() = default;
    OIOIOIManager & operator=(OIOIOIManager const &) = delete;
    OIOIOIManager & operator=(OIOIOIManager &&) noexcept = default;

    void setMainSeed(uint64_t seed) noexcept {
        source_generator = gen_type(seed);
    }

    [[nodiscard]] constexpr StreamType & stream() const noexcept {
        return curr_test->stream();
    }

    [[nodiscard]] constexpr gen_type & generator() const noexcept {
        return curr_test->generator();
    }

    void isEmpty() const {
        return curr_test == nullptr;
    }

    void skipTest() {
        curr_index.test++;
        clearStream();
    }

    void nextTest() {
        this->setTest(curr_index.test + 1, curr_index.suite);
    }

    void nextSuite() {
        curr_index = {0, curr_index.suite + 1};
        clearStream();
    }

    void setTest(unsigned test, unsigned suite) {
        curr_index = {test, suite};
        if(changeIfTaken()) { return; }
        changeToNewStream(getFilename());
    }

    void setTestSeed(uint64_t seed) {
        curr_test->generator() = gen_type(seed);
    }

    [[nodiscard]] std::string getFilename() const {
        if(curr_index.suite != 0U) {
            std::string suffix{};
            auto nr = curr_index.test - 1;
            constexpr unsigned SIZE = 'z' - 'a' + 1;
            do {
                suffix += 'a' + (nr % SIZE);
                nr /= SIZE;
            } while(nr != 0);
            std::reverse(std::begin(suffix), std::end(suffix));
            return abbr + std::to_string(curr_index.suite) + suffix + ".in";
        }
        return abbr + std::to_string(curr_index.test) + "ocen.in";
    }
};

} /* namespace test */

#endif /* TESTGEN_MANAGER_HPP_ */