#include "doctest.h"
#include <testgen/rand.hpp>
#include <type_traits>
using namespace test;

static_assert(std::is_copy_assignable_v<GeneratorWrapper<gen_type>>);
static_assert(std::is_copy_constructible_v<GeneratorWrapper<gen_type>>);
static_assert(std::is_move_assignable_v<GeneratorWrapper<gen_type>>);
static_assert(std::is_move_constructible_v<GeneratorWrapper<gen_type>>);
