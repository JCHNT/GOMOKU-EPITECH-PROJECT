#include "tss.hpp"
#include "engine/eval.hpp"
#include "engine/move_gen.hpp"
#include "engine/patterns.hpp"

namespace gomoku {

std::vector<Move> TSS::forcing_moves(Board& b, Color side, bool vct_mode) {
    std::vector<Move> fives, fours, threes;
    auto candidates = MoveGen::candidates(b);
    for (auto m : candidates) {
        b.make_move(m, side);
        auto counts = b.count_patterns(side);
        b.undo_move();
        if (counts[Patterns::FIVE] > 0)        fives.push_back(m);
        else if (counts[Patterns::OPEN_FOUR] > 0
              || counts[Patterns::CLOSED_FOUR] > 0) fours.push_back(m);
        else if (vct_mode && counts[Patterns::OPEN_THREE] > 0) threes.push_back(m);
    }
    fives.insert(fives.end(), fours.begin(), fours.end());
    fives.insert(fives.end(), threes.begin(), threes.end());
    return fives;
}

bool TSS::search(Board& b, Color side, Color root_side, int depth, bool vct_mode) {
    if (b.has_five(root_side)) return true;
    if (b.has_five(other(root_side))) return false;
    if (depth <= 0) return false;

    if (side == root_side) {
        for (auto m : forcing_moves(b, side, vct_mode)) {
            b.make_move(m, side);
            bool won = search(b, other(side), root_side, depth - 1, vct_mode);
            b.undo_move();
            if (won) return true;
        }
        return false;
    } else {
        auto moves = MoveGen::candidates(b);
        if (moves.empty()) return false;
        for (auto m : moves) {
            b.make_move(m, side);
            bool attacker_wins = search(b, other(side), root_side, depth - 1, vct_mode);
            b.undo_move();
            if (!attacker_wins) return false;
        }
        return true;
    }
}

bool TSS::find_vcf(Board& b, Color side, int max_depth, Move& out) {
    for (auto m : forcing_moves(b, side, false)) {
        b.make_move(m, side);
        bool won = search(b, other(side), side, max_depth - 1, false);
        b.undo_move();
        if (won) { out = m; return true; }
    }
    return false;
}

bool TSS::find_vct(Board& b, Color side, int max_depth, Move& out) {
    for (auto m : forcing_moves(b, side, true)) {
        b.make_move(m, side);
        bool won = search(b, other(side), side, max_depth - 1, true);
        b.undo_move();
        if (won) { out = m; return true; }
    }
    return false;
}

} // namespace gomoku
