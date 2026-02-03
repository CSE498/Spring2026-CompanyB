#!/bin/bash
set -e

g++ -std=c++20 -Wall -Wextra -I.. \
    -I/opt/homebrew/include \
    -L/opt/homebrew/lib \
    -o RobinHoodMapTest \
    RobinHoodMapTest.cpp \
    -lgtest -lgtest_main -pthread

./RobinHoodMapTest "$@"

g++ -std=c++20 -O2 -I.. -o RobinHoodMapBenchmark \
    RobinHoodMapBenchmark.cpp
./RobinHoodMapBenchmark "$@"

rm RobinHoodMapTest RobinHoodMapBenchmark