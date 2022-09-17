#include "test.hpp"
#include <rand.hpp>
#include <algorithm>
using namespace test;
using namespace std;

void test_standard() {    
    UniDist<int> dist(1,10);
    vector<int> V(1000);
    std::generate(begin(V), end(V), [gen = gen_type{12}, &dist]() mutable {return dist(gen);});
    for(auto v : V)
        assert(1 <= v && v <= 10);
    for(int i=1;i<=10;i++)
        assert(find(begin(V),end(V),i) != end(V));
}

void test_single() {
    UniDist<int> dist(7,7);
    gen_type gen{13};
    for(int i=1;i<=1000;i++)
        assert(dist(gen) == 7);
}

int main() {
    TEST_OK();
    return 0;
}