#ifndef TESTGEN_TESTING_HPP_
#define TESTGEN_TESTING_HPP_

#include "rand.hpp"

#include <memory>
#include <ostream>

namespace test {

struct testcase {
    std::unique_ptr<std::ostream> stream;
    gen_type generator;
};

template<typename TestcaseManager>
class Testing {
    TestcaseManager manager;
};

} /* namespace test */


#endif /* TESTGEN_TESTING_H_ */