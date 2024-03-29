#ifndef TESTGEN_TESTING_HPP_
#define TESTGEN_TESTING_HPP_

#include "assumptions.hpp"
#include "output.hpp"
#include "rand.hpp"

#include <iostream>

namespace test {

class DummyTestcase {};

template<typename TestcaseManagerT, typename TestcaseT = DummyTestcase, template<typename> typename AssumptionsManagerT = AssumptionManager>
class Testing : private TestcaseManagerT, public RngUtilities<Testing<TestcaseManagerT, TestcaseT, AssumptionsManagerT>> {
    template<typename T>
    auto generate(Generating<T> const & schema) {
        return schema.generate(generator());
    }

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
    decltype(auto) operator()(T const & t) { /* decltype(auto) does not decay static arrays to pointers */
        if constexpr(is_generating<T>::value) {
            return generate(t);
        } else {
            return t;
        }
    }

    template<typename T>
    Testing & operator<<(T const & out) {
        if constexpr(std::is_same_v<T, TestcaseT>) {
            if(!checkSoft(out)) {
                std::cerr << "Assumption failed for " << this->TestcaseManagerT::getFilename() << '\n';
                assume(false);
            }
        }
        output << (*this)(out);
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
        assume(checkHard(tc));
        return true;
    }
};

} /* namespace test */

#endif /* TESTGEN_TESTING_H_ */