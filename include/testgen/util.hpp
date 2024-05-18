#ifndef TESTGEN_UTIL_HPP_
#define TESTGEN_UTIL_HPP_

#include <cstdlib>

void assume(bool value) {
    if(!value) { exit(EXIT_FAILURE); }
}

#endif /* TESTGEN_UTIL_HPP_ */