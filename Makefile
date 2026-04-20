NAME     := pbrain-gomoku-ai
CXX      ?= g++
CXXFLAGS := -std=c++20 -O3 -march=native -DNDEBUG -pthread -Wall -Wextra -Werror -Isrc
LDFLAGS  := -pthread

ifeq ($(OS),Windows_NT)
    NAME := $(NAME).exe
    LDFLAGS += -static
    CXXFLAGS := $(filter-out -march=native,$(CXXFLAGS))
endif

SRC      := $(shell find src -name '*.cpp' 2>/dev/null)
OBJ      := $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(OBJ) -o $(NAME) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) tests/*.o

fclean: clean
	rm -f $(NAME) tests/gomoku_test tests/gomoku_test_bonus

re: fclean all

test:
	$(MAKE) -f tests/Makefile run

bonus:
	$(MAKE) -C bonus

debug: CXXFLAGS := -std=c++20 -O0 -g -pthread -Wall -Wextra -Isrc -fsanitize=address,undefined
debug: LDFLAGS += -fsanitize=address,undefined
debug: re

.PHONY: all re clean fclean test bonus debug
