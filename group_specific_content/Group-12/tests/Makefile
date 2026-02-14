CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -I..
LDFLAGS = -lgtest -lgtest_main -pthread

CXXFLAGS += -I/opt/homebrew/include
LDFLAGS += -L/opt/homebrew/lib

TARGET = RobinHoodMapTest
SRC = RobinHoodMapTest.cpp

.PHONY: all test clean

all: $(TARGET)
	./$(TARGET)

$(TARGET): $(SRC) ../RobinHoodMap.hpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)