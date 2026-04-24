#pragma once

#include "board/board.hpp"
#include "util/types.hpp"
#include <vector>

namespace gomoku {

class TSS {
public:
    static bool find_vcf(Board& b, Color side, int max_depth, Move& out);
    static bool find_vct(Board& b, Color side, int max_depth, Move& out);

private:
    static bool search(Board& b, Color side, Color root_side, int depth, bool vct_mode);
    static std::vector<Move> forcing_moves(Board& b, Color side, bool vct_mode);
};

} // namespace gomoku
