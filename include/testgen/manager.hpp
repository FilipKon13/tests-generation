#ifndef TESTGEN_MANAGER_HPP_
#define TESTGEN_MANAGER_HPP_

#include "rand.hpp"

#include <algorithm>
#include <iostream>
#include <unordered_map>

namespace test {

namespace detail {
struct index {
    unsigned test;
    unsigned suite;

    [[nodiscard]] bool operator==(index const & x) const {
        return test == x.test && suite == x.suite;
    }

    struct hash {
        [[nodiscard]] std::size_t operator()(index const & indx) const {
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
    void changeStream() {
        auto test_name = getFilename();
        if constexpr(Verbose == Verbocity::VERBOSE) {
            std::cerr << "Printing to: " << test_name << '\n';
        }
        auto it = cases.find(curr_index);
        if(it == cases.end()) {
            it = cases.try_emplace(curr_index,
                                   std::piecewise_construct,
                                   std::forward_as_tuple(std::move(test_name)),
                                   std::forward_as_tuple(getGeneratorForCurrentTest()))
                     .first;
        }
        curr_test = &it->second;
    }

    gen_type getGeneratorForCurrentTest() {
        auto const current_suite_nr = curr_index.suite;
        auto it = suite_generators.find(current_suite_nr);
        if(it == suite_generators.end()) {
            it = suite_generators.try_emplace(current_suite_nr, main_generator()).first;
        }
        return it->second.fork();
    }

    void clearStream() {
        curr_test = nullptr;
    }

    struct test_info : private std::pair<StreamType, gen_type> {
        using std::pair<StreamType, gen_type>::pair;
        [[nodiscard]] StreamType & stream() {
            return this->first;
        }
        [[nodiscard]] gen_type & generator() {
            return this->second;
        }
    };
    using index = detail::index;
    test_info * curr_test{nullptr};
    index curr_index;
    std::string abbr;
    std::unordered_map<index, test_info, index::hash> cases{};
    std::unordered_map<unsigned, gen_type> suite_generators{};
    gen_type main_generator;

public:
    explicit OIOIOIManager(std::string abbr, bool ocen = true, uint64_t seed = TESTGEN_SEED) :
      curr_index{0U, ocen ? 0U : 1U}, abbr{std::move(abbr)}, main_generator{seed} {}

    OIOIOIManager() = delete;
    OIOIOIManager(OIOIOIManager const &) = delete;
    OIOIOIManager(OIOIOIManager &&) noexcept = default;
    ~OIOIOIManager() = default;
    OIOIOIManager & operator=(OIOIOIManager const &) = delete;
    OIOIOIManager & operator=(OIOIOIManager &&) noexcept = default;

    void setMainSeed(uint64_t seed) noexcept {
        main_generator = gen_type(seed);
    }

    void setSuiteSeed(uint64_t seed) noexcept {
        suite_generators[curr_index.suite] = gen_type(seed);
    }

    void setTestSeed(uint64_t seed) noexcept {
        generator() = gen_type(seed);
    }

    [[nodiscard]] StreamType & stream() const {
        return curr_test->stream();
    }

    [[nodiscard]] gen_type & generator() const {
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
        changeStream();
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