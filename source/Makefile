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

# Pass NO_QT=1 in the command line to skip Qt and its GUI sources.
NO_QT ?= 0
ifeq ($(NO_QT),1)
  CMAKE_NO_QT := -DNO_QT=ON
else
  CMAKE_NO_QT :=
endif

.PHONY: default build debug opt quick grumpy clean

default: build

build:
	cmake -S . -B $(BUILD_DIR) $(CMAKE_NO_QT)
	cmake --build $(BUILD_DIR) --parallel

debug:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug $(CMAKE_NO_QT)
	cmake --build $(BUILD_DIR) --parallel

opt:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release $(CMAKE_NO_QT)
	cmake --build $(BUILD_DIR) --parallel

quick:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_CXX_FLAGS="-DNDEBUG" $(CMAKE_NO_QT)
	cmake --build $(BUILD_DIR) --parallel

grumpy:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_CXX_FLAGS="-Wconversion -Weffc++" $(CMAKE_NO_QT)
	cmake --build $(BUILD_DIR) --parallel

clean:
	rm -rf $(BUILD_DIR)
