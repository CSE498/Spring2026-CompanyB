#!/bin/bash
set -e

g++ -std=c++20 -Wall -Wextra -I.. \
    -I/opt/homebrew/include \
    -L/opt/homebrew/lib \
    -o RobinHoodMapTest \
    RobinHoodMapTest.cpp \
    -lgtest -lgtest_main -pthread

./RobinHoodMapTest "$@"

rm RobinHoodMapTest