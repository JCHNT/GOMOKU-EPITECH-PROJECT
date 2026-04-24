#include "engine/search.hpp"
#include "engine/eval.hpp"
#include "engine/move_gen.hpp"
#include <algorithm>

namespace gomoku {

static constexpr int STOP_CHECK_INTERVAL = 2048;

Move Search::go(Board& b, Color side, int max_depth) {
    nodes_ = 0;
    best_root_ = NO_MOVE;
    tt_.new_search();

    Move best = NO_MOVE;
    for (int d = 1; d <= max_depth; ++d) {
        if (tm_.soft_stop() && d > 1) break;

        auto moves = MoveGen::ordered_candidates(b, side);
        if (moves.empty()) break;

        int alpha = -WIN_SCORE * 2;
        const int beta = WIN_SCORE * 2;
        Move iter_best = moves.front();
        int iter_best_score = -WIN_SCORE * 2;

        for (auto m : moves) {
            if (tm_.should_stop()) break;
            b.make_move(m, side);
            int sc = -alphabeta(b, other(side), d - 1, -beta, -alpha, 1);
            b.undo_move();
            if (sc > iter_best_score) {
                iter_best_score = sc;
                iter_best = m;
                if (sc > alpha) alpha = sc;
            }
        }

        if (!tm_.should_stop() || best == NO_MOVE) {
            best = iter_best;
            best_root_ = iter_best;
        }
        if (iter_best_score >= WIN_SCORE) break;
    }
    return best;
}

int Search::alphabeta(Board& b, Color side, int depth, int alpha, int beta, int ply) {
    if ((++nodes_ & (STOP_CHECK_INTERVAL - 1)) == 0 && tm_.should_stop()) {
        return 0;
    }

    if (b.has_five(other(side))) return -WIN_SCORE + ply;
    if (depth <= 0) return Eval::score(b, side);

    auto moves = MoveGen::ordered_candidates(b, side);
    if (moves.empty()) return Eval::score(b, side);

    int best = -WIN_SCORE * 2;
    for (auto m : moves) {
        b.make_move(m, side);
        int sc = -alphabeta(b, other(side), depth - 1, -beta, -alpha, ply + 1);
        b.undo_move();
        if (tm_.should_stop()) return best > -WIN_SCORE * 2 ? best : 0;
        if (sc > best) best = sc;
        if (best > alpha) alpha = best;
        if (alpha >= beta) break;
    }
    return best;
}

} // namespace gomoku
