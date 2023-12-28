#ifndef TESTGEN_TESTING_HPP_
#define TESTGEN_TESTING_HPP_

#include "assumptions.hpp"
#include "output.hpp"
#include "rand.hpp"

#include <ostream>

namespace test {

template<typename TestcaseManager_t, typename Testcase_t = DummyTestcase, template<typename> typename AssumptionsManager_t = AssumptionManager>
class Testing : private TestcaseManager_t {
    template<typename T>
    auto generate(Generating<T> const & schema) {
        return schema.generate(TestcaseManager_t::generator());
    }

    Testcase_t update_testcase() {
        output.set(this->stream());
        Testcase_t T;
        if constexpr(has_gen_v<Testcase_t>) {
            T.gen = TestcaseManager_t::generator();
        }
        return T;
    }

    Output output;
    AssumptionsManager_t<Testcase_t> assumptions;

public:
    using TestcaseManager_t::TestcaseManager_t;

    Testing(const Testing &) = delete;
    Testing(Testing &&) = delete;
    Testing & operator=(const Testing &) = delete;
    Testing & operator=(Testing &&) = delete;
    ~Testing() = default;

    GeneratorWrapper<gen_type> generator() {
        return GeneratorWrapper<gen_type>{TestcaseManager_t::generator()};
    }

    Testcase_t nextSuite() {
        TestcaseManager_t::nextSuite();
        assumptions.resetSuite();
        assumptions.resetTest();
        return update_testcase();
    }

    Testcase_t nextTest() {
        TestcaseManager_t::nextTest();
        assumptions.resetTest();
        return update_testcase();
    }

    template<typename... T>
    Testcase_t test(T &&... args) {
        TestcaseManager_t::test(std::forward<T>(args)...);
        return update_testcase();
    }

    template<typename... T>
    void print(T const &... args) {
        output.dump_output((*this)(args)...);
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
        if constexpr(std::is_same_v<T, Testcase_t>) {
            assume(assumptions.check(out));
        }
        output << (*this)(out);
        return *this;
    }

    using assumption_t = typename AssumptionsManager_t<Testcase_t>::assumption_t;

    void globalAssumption(assumption_t fun) {
        assumptions.setGlobal(fun);
    }

    void suiteAssumption(assumption_t fun) {
        assumptions.setSuite(fun);
    }

    void testAssumption(assumption_t fun) {
        assumptions.setTest(fun);
    }
};

} /* namespace test */

#endif /* TESTGEN_TESTING_H_ */