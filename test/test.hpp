#ifndef TESTGEN_TEST_HPP_
#define TESTGEN_TEST_HPP_

#include <cassert>
#include <string_view>
#include <iostream>

void log_test(const std::string_view s) {
    std::clog << "\e[92m" << s << "\e[39m\n";
}

#define TEST_OK() log_test( __FILE__ " OK")

#endif /* TESTGEN_TEST_HPP_ */