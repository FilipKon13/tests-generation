#!/bin/bash

make clean

find . -type d -exec rm -rf {}/cmake_install.cmake {}/CMakeCache.txt {}/CTestTestfile.cmake {}/Testing {}/CMakeFiles \;