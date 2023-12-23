#include "test.hpp"
#include <rand.hpp>
#include <vector>
#include <functional>
using namespace test;

template<typename T>
class test_generating : Generating<T> {
    mutable std::function<T(gen_type&)> _fun;
public:
    test_generating(std::function<T(gen_type&)> fun) : _fun(fun) {}
    T generate(gen_type& gen) override {
        return _fun(gen);
    }
};

void test_generating_interface() {

}

int main() {
    test_generating_interface();

    TEST_OK();
}