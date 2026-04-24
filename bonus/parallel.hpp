#pragma once

#include "board/board.hpp"
#include "engine/search.hpp"
#include "engine/time_mgr.hpp"
#include "engine/tt.hpp"
#include "util/types.hpp"

namespace gomoku {

class ParallelSearch {
public:
    ParallelSearch(TranspositionTable& tt, TimeMgr& tm, int threads);
    Move go(Board& b, Color side, int max_depth);

private:
    TranspositionTable& tt_;
    TimeMgr& tm_;
    int threads_;
};

} // namespace gomoku
