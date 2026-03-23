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

<<<<<<< HEAD
CATCH_INC := ../third-party/Catch/single_include
QT_INC := ../third-party/qt/include
QT_FLAGS := $(shell pkg-config --cflags Qt6Widgets 2>/dev/null || pkg-config --cflags Qt5Widgets)
QT_LIBS  := $(shell pkg-config --libs   Qt6Widgets 2>/dev/null || pkg-config --libs   Qt5Widgets)

CXXFLAGS ?= -O2 -g
WARNFLAGS ?= -Wall -Wextra -Wpedantic
CPPFLAGS += -I$(SRC_DIR) -I$(CATCH_INC) -I$(QT_INC) $(QT_FLAGS)
LDFLAGS  ?=
LDLIBS   ?= $(QT_LIBS)

# Pick up .cpp from tests/
TEST_FILES := $(shell find $(TEST_DIR) -type f -name '*.cpp')

# Pick up headers from source/, then compile only matching .cpp next to them
SRC_HPP    := $(shell find $(SRC_DIR)  -type f -name '*.hpp')
SRC_FILES  := $(patsubst %.hpp,%.cpp,$(SRC_HPP))
SRC_FILES  := $(filter $(shell find $(SRC_DIR) -type f -name '*.cpp'),$(SRC_FILES))

OBJ_FILES := \
  $(patsubst $(TEST_DIR)/%,$(BUILD_DIR)/tests/%.o,$(TEST_FILES)) \
  $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/source/%.o,$(SRC_FILES))

DEP_FILES := $(OBJ_FILES:.o=.d)

.PHONY: all build test clean list
=======
# Pass NO_QT=1 in the command line to skip Qt and its GUI sources.
NO_QT ?= 0
ifeq ($(NO_QT),1)
  CMAKE_NO_QT := -DNO_QT=ON
else
  CMAKE_NO_QT :=
endif

.PHONY: all build test list
>>>>>>> origin/main

all: test

build:
	cmake -S ../source -B $(BUILD_DIR) -DBUILD_TESTS=ON $(CMAKE_NO_QT)
	cmake --build $(BUILD_DIR) --parallel

test: build
	$(BUILD_DIR)/tests

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
