#!/bin/bash


# for file in *.cpp *.hpp; do
find . -type f \( -name "*.cpp" -o -name "*.hpp" \) | while read -r file; do
    dir=$(dirname "$file")
    base=$(basename "$file")
    c++ $file -std=c++23 -Wall -Wextra -Wcast-align -Wnon-virtual-dtor -Woverloaded-virtual -pedantic -c -o "$dir/_$base".out
done