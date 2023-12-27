#ifndef TESTGEN_ASSUMPTIONS_HPP_
#define TESTGEN_ASSUMPTIONS_HPP_

#include <type_traits>

namespace test {

template<typename Testcase_t>
class AssumptionManager {
public:
    using assumption_t = bool (*)(Testcase_t const &);

private:
    static bool empty(Testcase_t const & /*unused*/) {
        return true;
    }
    assumption_t global = empty;
    assumption_t suite = empty;
    assumption_t test = empty;

public:
    void setGlobal(assumption_t fun) {
        global = fun;
    }
    void setSuite(assumption_t fun) {
        suite = fun;
    }
    void setTest(assumption_t fun) {
        test = fun;
    }
    void resetGlobal() {
        global = empty;
    }
    void resetSuite() {
        suite = empty;
    }
    void resetTest() {
        test = empty;
    }
    bool check(Testcase_t const & testcase) {
        return test(testcase) && suite(testcase) && global(testcase);
    }
};

class DummyTestcase {};

template<typename T, typename = void>
struct has_gen : std::false_type {};

template<typename T>
struct has_gen<T, std::void_t<decltype(std::declval<T>().gen)>> : std::true_type {};

template<class T>
inline constexpr bool has_gen_v = has_gen<T>::value;

} /* namespace test */

#endif /* TESTGEN_ASSUMPTIONS_HPP_ */