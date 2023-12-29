#include "doctest.h"
#include <type_traits>

#include <testgen/rand.hpp>
using namespace test;

static_assert(std::is_copy_assignable_v<GeneratorWrapper<gen_type>>);
static_assert(std::is_copy_constructible_v<GeneratorWrapper<gen_type>>);
static_assert(std::is_move_assignable_v<GeneratorWrapper<gen_type>>);
static_assert(std::is_move_constructible_v<GeneratorWrapper<gen_type>>);
