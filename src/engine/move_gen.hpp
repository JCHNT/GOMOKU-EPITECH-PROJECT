#pragma once

#include "board/board.hpp"
#include "util/types.hpp"
#include <vector>

namespace gomoku {

class MoveGen {
public:
    static std::vector<Move> candidates(const Board& b);
    static std::vector<Move> ordered_candidates(Board& b, Color to_move);
};

} // namespace gomoku
