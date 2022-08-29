#ifndef TESTGEN_TESTING_HPP_
#define TESTGEN_TESTING_HPP_

#include "rand.hpp"
#include "output.hpp"

#include <memory>
#include <ostream>
#include <string_view>

namespace test {

// testcase does not manage lifetime of ostream object -
// manager should be responsible for that
struct testcase {
    std::ostream * stream;
    gen_type generator;
};

template<typename TestcaseManager>
class Testing : public Output, private TestcaseManager {

    using TestcaseManager::TestcaseManager;

    Testing(const Testing&) = delete;
    Testing(Testing&&) = delete;
    Testing& operator=(const Testing&) = delete;
    Testing& operator=(Testing&&) = delete;

public:
    void ignoreSuite() {
        this->TestcaseManager::ignoreSuite();
        set(nullptr);
    }

    void nextSuite() {
        this->TestcaseManager::nextSuite();
        set(this->TestcaseManager::testcase().stream);
    }

    void nextTest() {
        this->TestcaseManager::nextTest();
        set(this->TestcaseManager::testcase().stream);
    }

    template<typename... T>
    void test(T&&... args) {
        this->TestcaseManager::test(std::forward<T>(args)...);
        set(this->TestcaseManager::testcase().stream);
    }

    template<typename... T>
    void print(T const &... args) { // TODO generate
        auto & generator = this->TestcaseManager::testcase().generator;
        dump_output(args...);
    }
};

} /* namespace test */


#endif /* TESTGEN_TESTING_H_ */