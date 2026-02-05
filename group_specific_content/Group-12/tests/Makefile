CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -I..
LDFLAGS = -lgtest -lgtest_main -pthread

CXXFLAGS += -I/opt/homebrew/include
LDFLAGS += -L/opt/homebrew/lib

TARGET = RobinHoodMapTest
SRC = RobinHoodMapTest.cpp

.PHONY: all test clean

all: $(TARGET)
	./$(TARGET)

$(TARGET): $(SRC) ../RobinHoodMap.hpp ../RobinHoodMap.cpp
	$(CXX) $(CXXFLAGS) -o $@ $(SRC) $(LDFLAGS)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)