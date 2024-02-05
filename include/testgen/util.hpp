#ifndef TESTGEN_UTIL_HPP_
#define TESTGEN_UTIL_HPP_

#ifdef TESTRUN

#include <stdexcept>
inline void assume(bool value) {
    if(!value) { throw std::runtime_error("Assumption failed!"); }
}

#else

#include <cstdlib>
#include <iostream>

inline void assume(bool value) {
    if(!value) { exit(EXIT_FAILURE); }
}

#endif

#endif /* TESTGEN_UTIL_HPP_ */