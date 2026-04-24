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

CXXFLAGS_BONUS := -std=c++20 -O2 -pthread -Wall -Wextra -DBONUS -Isrc -Itests -Ibonus
SRC_BONUS_FILES := $(wildcard bonus/*.cpp)
OBJ_TEST_BONUS  := $(SRC_TEST:.cpp=.bonus.o)
OBJ_LIB_BONUS   := $(SRC_LIB:.cpp=.bonus.o)
OBJ_BONUS_FILES := $(SRC_BONUS_FILES:.cpp=.bonus.o)

%.bonus.o: %.cpp
	$(CXX) $(CXXFLAGS_BONUS) -c $< -o $@

bonus_test: $(OBJ_LIB_BONUS) $(OBJ_TEST_BONUS) $(OBJ_BONUS_FILES)
	$(CXX) $^ -o tests/gomoku_test_bonus $(LDFLAGS)

run_bonus: bonus_test
	./tests/gomoku_test_bonus

clean_bonus:
	rm -f $(OBJ_LIB_BONUS) $(OBJ_TEST_BONUS) $(OBJ_BONUS_FILES) tests/gomoku_test_bonus

clean:
	rm -f $(OBJ_LIB) $(OBJ_TEST) $(TARGET)

.PHONY: run clean bonus_test run_bonus clean_bonus
