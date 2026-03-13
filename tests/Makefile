# Makefile for Catch2 unit tests
#
# Assumptions:
# - Production sources live under: source/
# - Tests live under: tests/ (and mirror source/ directory structure)
#
# Usage:
#   make              # build + run tests
#   make test         # run test
#   make build        # build test binary only
#   make clean        # remove build directory
#   make list         # show source and test files cmake will pick up

BUILD_DIR := ../build/tests/native

# Pass NO_QT=1 in the command line to skip Qt and its GUI sources.
NO_QT ?= 0
ifeq ($(NO_QT),1)
  CMAKE_NO_QT := -DNO_QT=ON
else
  CMAKE_NO_QT :=
endif

.PHONY: all build test clean list

all: test

build:
	cmake -S ../source -B $(BUILD_DIR) -DBUILD_TESTS=ON $(CMAKE_NO_QT)
	cmake --build $(BUILD_DIR) --parallel

test: build
	$(BUILD_DIR)/tests

clean:
	rm -rf $(BUILD_DIR)

list:
	@echo "Sources:"; \
	for hpp in $$(find ../source -type f -name '*.hpp' | sort); do \
		cpp="$${hpp%.hpp}.cpp"; \
		[ -f "$$cpp" ] && echo "$$cpp" || true; \
	done
	@echo
	@echo "Tests:"; find ../tests -type f -name '*.cpp' | sort
	@echo
	@echo "Objects:"; find $(BUILD_DIR) -type f -name '*.o' ! -path '*/_deps/*' 2>/dev/null | sort || echo "  (no object files: run make build first)"
