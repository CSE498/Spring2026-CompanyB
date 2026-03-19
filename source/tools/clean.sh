#!/bin/bash

echo -e " ==> Deleting all .out files <== \n"

# for file in *.out; do
find . -type f \( -name "*.out" \) | while read -r file; do
    echo "Deleting $file"
    rm "$file"
done