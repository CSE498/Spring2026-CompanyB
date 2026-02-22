# Source Makefile
#
# Usage:
#   make              # configure + build
#   make debug        # configure + build with debug symbols
#   make opt          # configure + build optimized
#   make quick        # configure + build fastest compile
#   make grumpy       # configure + build with extra warnings
#   make clean        # remove build directory

BUILD_DIR := ../build/native

.PHONY: default build debug opt quick grumpy clean

default: build

build:
	cmake -S . -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR) --parallel

debug:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug
	cmake --build $(BUILD_DIR) --parallel

opt:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BUILD_DIR) --parallel

quick:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_CXX_FLAGS="-DNDEBUG"
	cmake --build $(BUILD_DIR) --parallel

grumpy:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_CXX_FLAGS="-Wconversion -Weffc++"
	cmake --build $(BUILD_DIR) --parallel

clean:
	rm -rf $(BUILD_DIR)
