CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Wpedantic -fsanitize=undefined
INCLUDES = -I ../../../../source

SRC = main.cpp \
      ../../../../source/tools/BehaviorTree/*.cpp

TARGET = BehaviorTree

all:
	$(CXX) $(CXXFLAGS) $(SRC) $(INCLUDES) -o $(TARGET)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)