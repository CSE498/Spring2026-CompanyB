# Makefile for Catch2 unit tests
#
# Assumptions:
# - Production sources live under: source/
# - Tests live under: tests/ (and mirror source/ directory structure)
#
# Usage:
#   make            # build + run tests
#   make test       # run tests
#   make build      # build test binary only
#   make clean

SRC_DIR   := ../source
TEST_DIR  := ../tests
BUILD_DIR := ../tests/build/
BIN       := $(BUILD_DIR)/run_tests

CATCH_INC := ../third-party/Catch/single_include

CXXFLAGS ?= -O2 -g
WARNFLAGS ?= -Wall -Wextra -Wpedantic
CPPFLAGS += -I$(SRC_DIR) -I$(CATCH_INC)
LDFLAGS  ?=
LDLIBS   ?=

QMAKE := $(shell which qmake 2>/dev/null || which qmake6 2>/dev/null)
ifdef QMAKE
	QT_INC    := $(shell $(QMAKE) -query QT_INSTALL_HEADERS)
  	QT_LIB    := $(shell $(QMAKE) -query QT_INSTALL_LIBS)
  	CPPFLAGS  += -I$(QT_INC) -I$(QT_INC)/QtCore -I$(QT_INC)/QtGui -I$(QT_INC)/QtWidgets
  	ifeq ($(shell uname 2>/dev/null),Darwin)
    	LDLIBS  += -F$(QT_LIB) -framework QtCore -framework QtGui -framework QtWidgets
  	else
    	LDLIBS  += -L$(QT_LIB) -lQt6Core -lQt6Gui -lQt6Widgets
  	endif
	else
  	$(warning WARNING: QMake not found. Qt-dependent tests will fail.)
  	$(warning Add qmake to PATH, e.g.)
endif

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

all: test

build: $(BIN)

test: $(BIN)
	$(BIN)

# Link test runner
$(BIN): $(OBJ_FILES) | $(BUILD_DIR)
	$(CXX) -std=c++23 $(CXXFLAGS) $(WARNFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Compile source/ -> build/tests/source/...
$(BUILD_DIR)/source/%.o: $(SRC_DIR)/% | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) -std=c++23 $(CPPFLAGS) $(CXXFLAGS) $(WARNFLAGS) -MMD -MP -c $< -o $@

# Compile tests/ -> build/tests/tests/...
$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/% | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) -std=c++23 $(CPPFLAGS) $(CXXFLAGS) $(WARNFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Sanity helper
list:
	@echo "Sources:"; echo "$(SRC_FILES)" | tr ' ' '\n'
	@echo
	@echo "Tests:";   echo "$(TEST_FILES)" | tr ' ' '\n'
	@echo
	@echo "Objects:"; echo "$(OBJ_FILES)" | tr ' ' '\n'

clean:
	rm -rf build

# Auto-deps
-include $(DEP_FILES)
