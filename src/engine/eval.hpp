#pragma once

#include "board/board.hpp"
#include "util/types.hpp"

namespace gomoku {

class Eval {
public:
    static int score(const Board& b, Color c);
    static constexpr int weight(Patterns::Kind k);
};

} // namespace gomoku
