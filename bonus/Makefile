NAME     := ../pbrain-gomoku-ai
CXX      ?= g++
CXXFLAGS := -std=c++20 -O3 -march=native -DNDEBUG -DBONUS -pthread -Wall -Wextra -Werror -I../src -I.
LDFLAGS  := -pthread

ifeq ($(OS),Windows_NT)
    NAME := ../pbrain-gomoku-ai.exe
    LDFLAGS += -static
    CXXFLAGS := $(filter-out -march=native,$(CXXFLAGS))
endif

SRC_MAIN := $(shell find ../src -name '*.cpp')
SRC_BONUS := $(wildcard *.cpp)
OBJ      := $(SRC_MAIN:.cpp=.o) $(SRC_BONUS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(OBJ) -o $(NAME) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

.PHONY: all clean
