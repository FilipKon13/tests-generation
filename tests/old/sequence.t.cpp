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
    Sequence<int> s2({1,2,3});
    auto s = s1 + s2;
    assert(s.size() == 5 && s[0] == 2 && s[2] == 1);
    s1 += s2;

    assert(s == s1);
}

void test_print() {
    stringstream s{};
    s << Sequence<int>({1,2,3,4,5});

    assert(s.str() == "1 2 3 4 5");
}

void test_generation_no_indx() {
    Sequence<int> s(4, [i = 0] () mutable -> int {return ++i;});
    Sequence<int> exp({1,2,3,4});
    assert(s == exp);
}

void test_generation_indx() {
    Sequence<int> s(4, [](int x){return x;});
    Sequence<int> exp({0,1,2,3});
    assert(s == exp);
}

void test_generation() {
    test_generation_no_indx();
    test_generation_indx();
}

void test_uni_sequence() {
    UniSequence<int> seq(3, 1, 1);
    gen_type gen{};
    assert(seq.generate(gen) == Sequence<int>({1,1,1}));
    // cerr << UniSequence<float>(20,0,5).generate(gen) << '\n';
}

void test_finite_sequence() {
    gen_type gen{};
    {
        FiniteSequence seq(20, (int[]){10, 20, 30});
        auto res = seq.generate(gen);
        assert(res.size() == 20);
        for(auto v : res)   assert(v == 10 || v == 20 || v == 30);
        for(int i=1;i<=3;++i)   assert(find(res.begin(), res.end(), i*10) != res.end());
    }
    {
        FiniteSequence seq(20, {10, 20, 30});
        auto res = seq.generate(gen);
        assert(res.size() == 20);
        for(auto v : res)   assert(v == 10 || v == 20 || v == 30);
        for(int i=1;i<=3;++i)   assert(find(res.begin(), res.end(), i*10) != res.end());
    }
    {
        int arr[] = {10, 20, 30};
        FiniteSequence seq(20, arr);
        auto res = seq.generate(gen);
        assert(res.size() == 20);
        for(auto v : res)   assert(v == 10 || v == 20 || v == 30);
        for(int i=1;i<=3;++i)   assert(find(res.begin(), res.end(), i*10) != res.end());
    }
    {
        auto res = FiniteSequence(5, {1.0, 2.0, 3.0}).generate(gen).front();
        static_assert(std::is_same_v<decltype(res), double>);
    }
}

int main() {
    test_construction();
    test_add();
    test_print();
    test_generation();
    test_uni_sequence();

    TEST_OK();
    return 0;
}