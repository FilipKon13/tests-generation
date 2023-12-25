#ifndef TESTGEN_UTIL_HPP_
#define TESTGEN_UTIL_HPP_

#include <cstdlib>
#include <iostream>

inline void assume(bool value) {
    if(!value) {
        std::cerr << "Assumption failed!\n";
        exit(EXIT_FAILURE);
    }
}

#endif /* TESTGEN_UTIL_HPP_ */