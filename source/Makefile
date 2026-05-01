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

ifneq ($(strip $(TARGET_MAIN)),)
	CMAKE_TARGET_MAIN := -DTARGET_MAIN=$(TARGET_MAIN)
else
	CMAKE_TARGET_MAIN :=
endif

.PHONY: default build debug opt quick grumpy

default: build

build:
	cmake -S . -B $(BUILD_DIR) $(CMAKE_NO_QT) $(CMAKE_TARGET_MAIN)
	cmake --build $(BUILD_DIR) --parallel

debug:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug $(CMAKE_NO_QT) $(CMAKE_TARGET_MAIN)
	cmake --build $(BUILD_DIR) --parallel

opt:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release $(CMAKE_NO_QT) $(CMAKE_TARGET_MAIN)
	cmake --build $(BUILD_DIR) --parallel

quick:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_CXX_FLAGS="-DNDEBUG" $(CMAKE_NO_QT) $(CMAKE_TARGET_MAIN)
	cmake --build $(BUILD_DIR) --parallel

grumpy:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_CXX_FLAGS="-Wconversion -Weffc++" $(CMAKE_NO_QT) $(CMAKE_TARGET_MAIN)
	cmake --build $(BUILD_DIR) --parallel

# Build and run a benchmark main file from benchmarking/groups/
# Usage:
#   make benchmark BENCH_FILE=example_1.cpp
#   make benchmark BENCH_FILE=groups/example_1.cpp
.PHONY: benchmark
benchmark:
	@if [ -z "$(BENCH_FILE)" ]; then \
		echo "Usage: make benchmark BENCH_FILE=<file_name.cpp>"; \
		exit 1; \
	fi
	@BENCH_PATH="../benchmarking/groups/$(BENCH_FILE)"; \
	if [ ! -f "$$BENCH_PATH" ] && echo "$(BENCH_FILE)" | grep -q "groups/"; then \
		BENCH_PATH="../benchmarking/$(BENCH_FILE)"; \
	fi; \
	if [ ! -f "$$BENCH_PATH" ]; then \
		echo "Benchmark file not found: ../benchmarking/groups/$(BENCH_FILE)"; \
		exit 1; \
	fi; \
	cmake -S . -B ../build/benchmark -DCMAKE_BUILD_TYPE=Release -DNO_QT=ON -DTARGET_MAIN="$$BENCH_PATH" -DCMAKE_CXX_FLAGS="-O3 -DBENCHMARK_MODE"
	cmake --build ../build/benchmark --parallel
	../build/benchmark/app
