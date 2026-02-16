CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -I..
LDFLAGS_GTEST = -lgtest -lgtest_main -pthread

CATCH_FLAGS = -I../../../third-party/Catch/single_include

CXXFLAGS += -I/opt/homebrew/include
LDFLAGS_GTEST += -L/opt/homebrew/lib

TARGET_MAP = RobinHoodMapTest
TARGET_PARSER = ExpressionParserTest

.PHONY: all test clean parser_test map_test

# Build both by default
all: $(TARGET_MAP) $(TARGET_PARSER)

# Build and run tests for RobinHoodMap
$(TARGET_MAP): RobinHoodMapTest.cpp ../RobinHoodMap.hpp
	$(CXX) $(CXXFLAGS) -o $@ RobinHoodMapTest.cpp $(LDFLAGS_GTEST)

map_test: $(TARGET_MAP)
	./$(TARGET_MAP)

# Build and run tests for ExpressionParser
$(TARGET_PARSER): ExpressionParserTest.cpp ../ExpressionParser.cpp ../ExpressionParser.hpp
	$(CXX) $(CXXFLAGS) $(CATCH_FLAGS) -o $@ ExpressionParserTest.cpp ../ExpressionParser.cpp

parser_test: $(TARGET_PARSER)
	./$(TARGET_PARSER)

clean:
	rm -f $(TARGET_MAP) $(TARGET_PARSER)