#pragma once

#include "board/board.hpp"
#include "engine/time_mgr.hpp"
#include "engine/tt.hpp"
#include "util/types.hpp"

namespace gomoku {

class Search {
public:
    Search(TranspositionTable& tt, TimeMgr& tm) : tt_(tt), tm_(tm) {}

    Move go(Board& b, Color side, int max_depth);

    uint64_t nodes() const { return nodes_; }

private:
    int alphabeta(Board& b, Color side, int depth, int alpha, int beta, int ply);

    TranspositionTable& tt_;
    TimeMgr& tm_;
    uint64_t nodes_ = 0;
    Move best_root_ = NO_MOVE;
};

} // namespace gomoku
