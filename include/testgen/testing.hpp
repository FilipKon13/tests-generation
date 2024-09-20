#ifndef TESTGEN_TESTING_HPP_
#define TESTGEN_TESTING_HPP_

#include "assumptions.hpp"
#include "output.hpp"
#include "rand.hpp"

#include <iostream>

namespace test {

template<typename TestcaseManagerT, typename TestcaseT = std::false_type, template<typename> typename AssumptionsManagerT = AssumptionManager>
class Testing : private TestcaseManagerT, public RngUtilities<Testing<TestcaseManagerT, TestcaseT, AssumptionsManagerT>> {
    TestcaseT updateTestcase() {
        output.set(this->stream());
        return TestcaseT{};
    }

    Output output;
    AssumptionsManagerT<TestcaseT> assumptions;

public:
    using TestcaseManagerT::TestcaseManagerT;

    Testing(const Testing &) = delete;
    Testing(Testing &&) = delete;
    Testing & operator=(const Testing &) = delete;
    Testing & operator=(Testing &&) = delete;
    ~Testing() = default;

    GeneratorWrapper<gen_type> generator() {
        return GeneratorWrapper<gen_type>{TestcaseManagerT::generator()};
    }

    void skipTest() {
        TestcaseManagerT::skipTest();
        assumptions.resetTest();
    }

    void nextSuite() {
        TestcaseManagerT::nextSuite();
        assumptions.resetSuite();
        assumptions.resetTest();
    }

    TestcaseT getTest() {
        TestcaseManagerT::nextTest();
        assumptions.resetTest();
        return updateTestcase();
    }

    void nextTest() {
        TestcaseManagerT::nextTest();
        assumptions.resetTest();
        updateTestcase();
    }

    // setTest(test_nr, suite), e.g. zad2c = (3, 2), 4ocen = (4, 0)
    // resets suite and test assumptions!
    template<typename T, typename U>
    TestcaseT setTest(T test_nr, U suite) {
        TestcaseManagerT::setTest(test_nr, suite);
        assumptions.resetSuite();
        assumptions.resetTest();
        return updateTestcase();
    }

    template<typename T>
    auto generateFromSchema(Generating<T> const & schema) {
        return schema.generate(generator());
    }

    template<typename T>
    Testing & operator<<(T const & out) {
        if constexpr(std::is_same_v<T, TestcaseT>) {
            if(!checkSoft(out)) {
                std::cerr << "Assumption failed for " << this->TestcaseManagerT::getFilename() << '\n';
                assume(false);
            }
        }
        if constexpr(is_generating<T>::value) {
            output << generateFromSchema(out);
        } else {
            output << out;
        }
        return *this;
    }

    template<typename AssT>
    void assumptionGlobal(AssT && fun) {
        assumptions.setGlobal(std::forward<AssT>(fun));
    }

    template<typename AssT>
    void assumptionSuite(AssT && fun) {
        assumptions.setSuite(std::forward<AssT>(fun));
    }

    template<typename AssT>
    void assumptionTest(AssT && fun) {
        assumptions.setTest(std::forward<AssT>(fun));
    }

    bool checkSoft(TestcaseT const & tc) {
        return assumptions.check(tc);
    }

    bool checkHard(TestcaseT const & tc) {
        assume(checkSoft(tc));
        return true;
    }
};

} /* namespace test */

#endif /* TESTGEN_TESTING_H_ */