CXX      ?= g++
CXXFLAGS := -std=c++20 -O2 -pthread -Wall -Wextra -Isrc -Itests
LDFLAGS  := -pthread

SRC_LIB  := $(shell find src -name '*.cpp' ! -name 'main.cpp' 2>/dev/null)
SRC_TEST := $(wildcard tests/test_*.cpp)

OBJ_LIB  := $(SRC_LIB:.cpp=.o)
OBJ_TEST := $(SRC_TEST:.cpp=.o)

TARGET := tests/gomoku_test

$(TARGET): $(OBJ_LIB) $(OBJ_TEST)
	$(CXX) $(OBJ_LIB) $(OBJ_TEST) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ_LIB) $(OBJ_TEST) $(TARGET)

.PHONY: run clean
