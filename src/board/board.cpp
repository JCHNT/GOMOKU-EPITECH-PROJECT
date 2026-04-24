#include "board/board.hpp"
#include <cstring>

namespace gomoku {

Board::Board() : hash_(0) {
    std::memset(cells_, 0, sizeof(cells_));
    std::memset(lines_, 0, sizeof(lines_));
    history_.reserve(BOARD_CELLS);
}

int Board::line_index(LineKind kind, int x, int y) {
    switch (kind) {
        case LINE_ROW:  return y;
        case LINE_COL:  return x;
        case LINE_DIAG: return x - y + (BOARD_SIZE - 1);
        case LINE_ANTI: return x + y;
    }
    return 0;
}

int Board::line_pos(LineKind kind, int x, int y) {
    switch (kind) {
        case LINE_ROW:  return x;
        case LINE_COL:  return y;
        case LINE_DIAG: return x;  // on diag: x - y = const, so x identifies cell
        case LINE_ANTI: return x;  // on anti-diag: x + y = const, so x identifies cell
    }
    return 0;
}

void Board::toggle_lines(Move m, Color c) {
    const int ci = (c == BLACK) ? 0 : 1;
    for (int k = 0; k < 4; ++k) {
        auto kind = static_cast<LineKind>(k);
        const int idx = line_index(kind, m.x, m.y);
        const int pos = line_pos(kind, m.x, m.y);
        lines_[ci][k][idx] ^= (1u << pos);
    }
}

bool Board::make_move(Move m, Color c) {
    if (!in_bounds(m)) return false;
    if (cells_[m.y][m.x] != EMPTY) return false;
    cells_[m.y][m.x] = c;
    hash_ ^= Zobrist::instance().key(c, m.x, m.y);
    toggle_lines(m, c);
    history_.push_back({m, c});
    return true;
}

void Board::undo_move() {
    if (history_.empty()) return;
    const auto entry = history_.back();
    history_.pop_back();
    cells_[entry.move.y][entry.move.x] = EMPTY;
    hash_ ^= Zobrist::instance().key(entry.color, entry.move.x, entry.move.y);
    toggle_lines(entry.move, entry.color);
}

} // namespace gomoku
