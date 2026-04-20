#pragma once

#include <array>
#include <cstdint>

namespace gomoku {

constexpr int BOARD_SIZE = 20;
constexpr int BOARD_CELLS = BOARD_SIZE * BOARD_SIZE;

enum Color : uint8_t {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2,
};

struct Move {
    int8_t x;
    int8_t y;
    bool operator==(const Move& o) const { return x == o.x && y == o.y; }
};

constexpr Move NO_MOVE = { -1, -1 };

constexpr int WIN_SCORE = 10'000'000;

inline Color other(Color c) { return c == BLACK ? WHITE : BLACK; }
inline int cell_index(int x, int y) { return y * BOARD_SIZE + x; }

} // namespace gomoku
