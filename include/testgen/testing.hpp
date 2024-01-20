#ifndef TESTGEN_TESTING_HPP_
#define TESTGEN_TESTING_HPP_

#include "assumptions.hpp"
#include "output.hpp"
#include "rand.hpp"

#include <ostream>

namespace test {

template<typename TestcaseManagerT, typename TestcaseT = DummyTestcase, template<typename> typename AssumptionsManagerT = AssumptionManager>
class Testing : private TestcaseManagerT {
    template<typename T>
    auto generate(Generating<T> const & schema) {
        return schema.generate(TestcaseManagerT::generator());
    }

    TestcaseT updateTestcase() {
        output.set(this->stream());
        TestcaseT T{};
        if constexpr(has_gen_v<TestcaseT>) {
            T.gen = TestcaseManagerT::generator();
        }
        return T;
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

    TestcaseT nextSuite() {
        TestcaseManagerT::nextSuite();
        assumptions.resetSuite();
        assumptions.resetTest();
        return updateTestcase();
    }

    TestcaseT nextTest() {
        TestcaseManagerT::nextTest();
        assumptions.resetTest();
        return updateTestcase();
    }

    template<typename... T>
    TestcaseT test(T &&... args) {
        TestcaseManagerT::test(std::forward<T>(args)...);
        return updateTestcase();
    }

    template<typename... T>
    void print(T const &... args) {
        output.dumpOutput((*this)(args)...);
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
    Testing & operator<<(const T & out) {
        if constexpr(std::is_same_v<T, TestcaseT>) {
            assume(assumptions.check(out));
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
};

class TestcaseBase {
public:
    GeneratorWrapper<gen_type> gen;
};

} /* namespace test */

#endif /* TESTGEN_TESTING_H_ */