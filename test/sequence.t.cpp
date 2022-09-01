#include "test.hpp"
#include <sequence.hpp>
#include <iostream>
#include <sstream>
using namespace test;
using namespace std;

void test_construction() {
    Sequence<int> s(3, 5);

    assert(s.size() == 3);
    assert(s[0] == 5 && s[1] == 5 && s[2] == 5);
}

void test_add() {
    Sequence<int> s1(2,2);
    Sequence<int> s2{1,2,3};
    auto s = s1 + s2;
    assert(s.size() == 5 && s[0] == 2 && s[2] == 1);
    s1 += s2;

    assert(s == s1);
}

void test_print() {
    stringstream s{};
    s << Sequence<int>{1,2,3,4,5};

    assert(s.str() == "1 2 3 4 5");
}

void test_generation_no_indx() {
    // Sequence<int> s(4, [i = 0] () mutable -> int {return ++i;});
    // Sequence<int> exp{1,2,3,4};
    // assert(s == exp);
}

void test_generation_indx() {
    Sequence<int> s(4, [](unsigned x){return x;});
    Sequence<int> exp{0,1,2,3};
    assert(s == exp);
}

void test_generation() {
    test_generation_no_indx();
    test_generation_indx();
}

int main() {
    test_construction();
    test_add();
    test_print();
    test_generation();

    TEST_OK();
    return 0;
}