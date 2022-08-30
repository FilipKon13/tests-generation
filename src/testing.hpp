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

    template<typename T>
    struct is_generating
    {
    private:
        template<typename V>
        static decltype(static_cast<const Generating<V>&>(std::declval<T>()), std::true_type{})
        helper(const Generating<V>&);
        
        static std::false_type helper(...); /* fallback */
    public:
        static constexpr bool value = decltype(helper(std::declval<T>()))::value;
    };

    template<typename T>
    T generate(Generating<T> const & schema) {
        return schema.generate(this->current().generator);
    }

public:
    using TestcaseManager::TestcaseManager;

    Testing(const Testing&) = delete;
    Testing(Testing&&) = delete;
    Testing& operator=(const Testing&) = delete;
    Testing& operator=(Testing&&) = delete;

    void nextSuite() {
        this->TestcaseManager::nextSuite();
        set(this->current().stream);
    }

    void nextTest() {
        this->TestcaseManager::nextTest();
        set(this->current().stream);
    }

    template<typename... T>
    void test(T&&... args) {
        this->TestcaseManager::test(std::forward<T>(args)...);
        set(this->current().stream);
    }

    template<typename... T>
    void print(T const &... args) {
        dump_output((*this)(args)...);
    }

    template<typename T>
    decltype(auto) operator()(T const & t) { /* decltype(auto) does not decay static arrays to pointers */
        if constexpr (is_generating<T>::value) {
            return generate(t);
        } else {
            return t;
        }
    }

    template<typename T>
    Testing& operator<<(const T & out) {
        static_cast<Output&>(*this) << (*this)(out);
        return * this;
    }
};

} /* namespace test */


#endif /* TESTGEN_TESTING_H_ */