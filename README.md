# TestGen

Single-header library for generating test files.

## Utilities

## Examples

## How to use it

## How it works

### Discussion about randomness
It is desirable that tests generated locally are identical to these generated when grading submission i.e. should not depend on external state or platform. Therefore we can expect pseudorandomness at best and this is what this library provides.

Header `<random>` is tricky in terms of cross-platformness. It turns out that standard lefts many aspects unspecified (e.g. implementation of `std::uniform_distribution` or `std::random_shuffle`) and using it with set seed only guarantees same results while using same implementation of standard library.

To address this issue, we use [xoshiro256++](https://prng.di.unimi.it/) engine for pseudorandom numbers and reimplement most usefull and common algorithms.

## Customization

## Installation

Just download `testgen.hpp` file from this repo and `#include` it.

However, we recommend cloning this repo and running tests anyway. To do this after cloning enter repo directory and run following command:

    make test

Do that to make sure that pseudorandomness works as intended. If tests fail two things are likely: current version of library depends on some implementation detail that standard left unspecified or your environment is not standard-compliant. Whatever the case is, you can try to solve this by providing your own `gen_type` type (more about it in Customization section) and running

    make __gen_tests

Be aware that this overwrites tests provided by the library. After that, by running

    make test

on your local and target (judge's) environment you can test their equivalence.