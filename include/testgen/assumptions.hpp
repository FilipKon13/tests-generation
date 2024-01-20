#ifndef TESTGEN_ASSUMPTIONS_HPP_
#define TESTGEN_ASSUMPTIONS_HPP_

#include <functional>
#include <type_traits>

namespace test {

template<typename TestcaseT>
class AssumptionManager {
public:
    // using assumption_t = bool (*)(TestcaseT const &);
    using assumption_t = std::function<bool(TestcaseT const &)>;

private:
    static bool empty(TestcaseT const & /*unused*/) {
        return true;
    }
    assumption_t global = empty;
    assumption_t suite = empty;
    assumption_t test = empty;

public:
    template<typename AssT>
    void setGlobal(AssT && fun) {
        global = std::forward<AssT>(fun);
    }
    template<typename AssT>
    void setSuite(AssT && fun) {
        suite = std::forward<AssT>(fun);
    }
    template<typename AssT>
    void setTest(AssT && fun) {
        test = std::forward<AssT>(fun);
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
    bool check(TestcaseT const & testcase) {
        return test(testcase) && suite(testcase) && global(testcase);
    }
};

template<typename Fun1T, typename Fun2T>
auto operator&&(Fun1T && fun1, Fun2T && fun2) {
    return [f1 = std::forward<Fun1T>(fun1), f2 = std::forward<Fun2T>(fun2)](auto const & testcase) mutable {
        return f1(testcase) && f2(testcase);
    };
}

template<typename Fun1T, typename Fun2T>
auto operator||(Fun1T && fun1, Fun2T && fun2) {
    return [f1 = std::forward<Fun1T>(fun1), f2 = std::forward<Fun2T>(fun2)](auto const & testcase) mutable {
        return f1(testcase) || f2(testcase);
    };
}

class DummyTestcase {};

template<typename T, typename = void>
struct has_gen : std::false_type {};

template<typename T>
struct has_gen<T, std::void_t<decltype(std::declval<T>().gen)>> : std::true_type {};

template<class T>
inline constexpr bool has_gen_v = has_gen<T>::value;    //NOLINT(readability-identifier-naming)

} /* namespace test */

#endif /* TESTGEN_ASSUMPTIONS_HPP_ */