#pragma once

#include "board/zobrist.hpp"
#include "util/types.hpp"
#include <cstdint>
#include <vector>

namespace gomoku {

class Board {
public:
    enum LineKind : uint8_t {
        LINE_ROW = 0,   // 20 rows
        LINE_COL = 1,   // 20 cols
        LINE_DIAG = 2,  // 39 main diagonals  (indexed by x - y + (N-1))
        LINE_ANTI = 3,  // 39 anti-diagonals (indexed by x + y)
    };

    static constexpr int N_ROWS  = BOARD_SIZE;
    static constexpr int N_COLS  = BOARD_SIZE;
    static constexpr int N_DIAGS = 2 * BOARD_SIZE - 1;  // 39
    static constexpr int N_ANTIS = 2 * BOARD_SIZE - 1;  // 39

    Board();

    bool make_move(Move m, Color c);
    void undo_move();

    Color at(int x, int y) const { return static_cast<Color>(cells_[y][x]); }
    uint64_t hash() const { return hash_; }
    int stone_count() const { return static_cast<int>(history_.size()); }
    Move last_move() const {
        return history_.empty() ? NO_MOVE : history_.back().move;
    }

    uint32_t line_mask(Color c, LineKind kind, int idx) const {
        return lines_[c == BLACK ? 0 : 1][kind][idx];
    }

    static int line_pos(LineKind kind, int x, int y);
    static int line_index(LineKind kind, int x, int y);

    static bool in_bounds(Move m) {
        return m.x >= 0 && m.x < BOARD_SIZE && m.y >= 0 && m.y < BOARD_SIZE;
    }

private:
    struct HistoryEntry {
        Move move;
        Color color;
    };

    void toggle_lines(Move m, Color c);

    uint8_t cells_[BOARD_SIZE][BOARD_SIZE];
    uint64_t hash_;
    uint32_t lines_[2][4][N_DIAGS];  // [color][kind][line_index]
    std::vector<HistoryEntry> history_;
};

} // namespace gomoku
