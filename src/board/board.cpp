#include "board/board.hpp"
#include <cstring>

namespace gomoku {

Board::Board() : hash_(0) {
    std::memset(cells_, 0, sizeof(cells_));
    history_.reserve(BOARD_CELLS);
}

bool Board::make_move(Move m, Color c) {
    if (!in_bounds(m)) return false;
    if (cells_[m.y][m.x] != EMPTY) return false;
    cells_[m.y][m.x] = c;
    hash_ ^= Zobrist::instance().key(c, m.x, m.y);
    history_.push_back({m, c});
    return true;
}

void Board::undo_move() {
    if (history_.empty()) return;
    const auto entry = history_.back();
    history_.pop_back();
    cells_[entry.move.y][entry.move.x] = EMPTY;
    hash_ ^= Zobrist::instance().key(entry.color, entry.move.x, entry.move.y);
}

} // namespace gomoku
