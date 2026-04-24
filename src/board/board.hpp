#pragma once

#include "board/zobrist.hpp"
#include "util/types.hpp"
#include <cstdint>
#include <vector>

namespace gomoku {

class Board {
public:
    Board();

    bool make_move(Move m, Color c);
    void undo_move();

    Color at(int x, int y) const {
        return static_cast<Color>(cells_[y][x]);
    }
    uint64_t hash() const { return hash_; }
    int stone_count() const { return static_cast<int>(history_.size()); }

    Move last_move() const {
        return history_.empty() ? NO_MOVE : history_.back().move;
    }

    static bool in_bounds(Move m) {
        return m.x >= 0 && m.x < BOARD_SIZE && m.y >= 0 && m.y < BOARD_SIZE;
    }

private:
    struct HistoryEntry {
        Move move;
        Color color;
    };

    uint8_t cells_[BOARD_SIZE][BOARD_SIZE];
    uint64_t hash_;
    std::vector<HistoryEntry> history_;
};

} // namespace gomoku
