#pragma once

#include "util/types.hpp"
#include <cstdint>

namespace gomoku {

class Zobrist {
public:
    static const Zobrist& instance();

    uint64_t key(Color c, int x, int y) const {
        return keys_[c == BLACK ? 0 : 1][cell_index(x, y)];
    }

private:
    Zobrist();
    uint64_t keys_[2][BOARD_CELLS];
};

} // namespace gomoku
