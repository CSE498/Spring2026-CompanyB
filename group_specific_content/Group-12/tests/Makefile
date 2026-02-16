CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -I..
LDFLAGS = -pthread

CXXFLAGS += -I/opt/homebrew/include
LDFLAGS += -L/opt/homebrew/lib

TARGET = RobinHoodMapTest
SRC = RobinHoodMapTest.cpp

.PHONY: all test test-static-assert clean

all: $(TARGET)
	./$(TARGET)

$(TARGET): $(SRC) ../RobinHoodMap.hpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

test: $(TARGET) test-static-assert
	./$(TARGET)

test-static-assert:
	@echo "Testing static_assert rejects non-default-constructible V..."
	@if $(CXX) $(CXXFLAGS) -c RobinHoodMapAssertTest.cpp -o /dev/null 2>/dev/null; then \
		echo "FAIL: Compilation should have failed"; exit 1; \
	else \
		echo "PASS: Compilation correctly rejected"; \
	fi

clean:
	rm -f $(TARGET)