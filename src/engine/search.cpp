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
    if ((++nodes_ & (STOP_CHECK_INTERVAL - 1)) == 0 && tm_.should_stop()) return 0;

    const int alpha_orig = alpha;

    TTEntry tt_e;
    Move tt_move = NO_MOVE;
    if (tt_.probe(b.hash(), tt_e) && tt_e.depth >= depth) {
        if (tt_e.flag == TT_EXACT) return tt_e.score;
        if (tt_e.flag == TT_LOWER && tt_e.score >= beta) return tt_e.score;
        if (tt_e.flag == TT_UPPER && tt_e.score <= alpha) return tt_e.score;
        tt_move = tt_e.best;
    }

    if (b.has_five(other(side))) return -WIN_SCORE + ply;
    if (depth <= 0) return Eval::score(b, side);

    auto moves = MoveGen::ordered_candidates(b, side);
    if (moves.empty()) return Eval::score(b, side);

    if (!(tt_move == NO_MOVE)) {
        auto it = std::find(moves.begin(), moves.end(), tt_move);
        if (it != moves.end()) std::iter_swap(moves.begin(), it);
    }

    int best = -WIN_SCORE * 2;
    Move best_move = moves.front();
    bool first = true;
    for (auto m : moves) {
        b.make_move(m, side);
        int sc;
        if (first) {
            sc = -alphabeta(b, other(side), depth - 1, -beta, -alpha, ply + 1);
        } else {
            sc = -alphabeta(b, other(side), depth - 1, -alpha - 1, -alpha, ply + 1);
            if (sc > alpha && sc < beta) {
                sc = -alphabeta(b, other(side), depth - 1, -beta, -alpha, ply + 1);
            }
        }
        b.undo_move();
        if (tm_.should_stop()) return best > -WIN_SCORE * 2 ? best : 0;
        if (sc > best) { best = sc; best_move = m; }
        if (best > alpha) alpha = best;
        if (alpha >= beta) break;
        first = false;
    }

    TTEntry store{};
    store.key = b.hash();
    store.depth = static_cast<int16_t>(depth);
    store.score = best;
    store.best = best_move;
    if (best <= alpha_orig)  store.flag = TT_UPPER;
    else if (best >= beta)   store.flag = TT_LOWER;
    else                     store.flag = TT_EXACT;
    tt_.store(store);
    return best;
}

} // namespace gomoku
