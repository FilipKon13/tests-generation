# TestGen

Single-header library for generating test files.

## Utilities

## Examples

## How to use it

## How it works

### Some design choices

To prevent user from accidentally taking copy of RNG assigned to particular testcase (which could lead to generating same bits over and over) `Testing::generator` method returns wrapper around real reference to RNG object. Client-site usage is basically the same as if method returned normal reference (using `operator()` to generate numbers) but there is only one way to assing return value i.e.

    Testing</* */> test;
    auto gen = test.generator(); // Only proper way at the moment
    auto & gen = test.generator(); // Does not compile, therefore ok
    /* If method returned reference it would be: */
    auto & gen = test.generator(); // OK
    auto gen = test.generator(); // BAD, we create a copy from reference

It is more verbose if one wanted to use real types (`GeneratorWrapper<gen_type>` vs. `gen_type`) so usage of `auto` identifier is encouraged. Another solution to this problem would be making RNG non-copyable, but that would be weird and problematic.

### Discussion about randomness

It is desirable that tests generated locally are identical to these generated when grading submission i.e. should not depend on external state or platform. Therefore we can expect pseudorandomness at best and this is what this library provides.

Header `<random>` is tricky in terms of cross-platformness. It turns out that standard lefts many aspects unspecified (e.g. implementation of `std::uniform_distribution` or `std::random_shuffle`) and using it with set seed only guarantees same results while using same implementation of standard library.

To address this issue, we use [xoshiro256++](https://prng.di.unimi.it/) engine for pseudorandom numbers and reimplement most usefull and common algorithms.

## Customization

## Installation

Just download `testgen.hpp` file from this repo and `#include` it.

However, we recommend cloning this repo and running tests anyway. To do this after cloning enter repo directory and run following command:

    make test

Do that to make sure that pseudorandomness works as intended. If tests fail two things are likely: current version of library depends on some implementation detail that standard left unspecified or your environment is not standard-compliant. Whatever the case is, you can try to bypass this problem by running

    make gen_tests

Be aware that this overwrites tests provided by the library. After that, by running

    make test

on your local and target (judge's) environment you can test their equivalence.
