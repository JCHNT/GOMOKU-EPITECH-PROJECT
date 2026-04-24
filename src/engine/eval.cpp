#include "engine/eval.hpp"

namespace gomoku {

constexpr int Eval::weight(Patterns::Kind k) {
    switch (k) {
        case Patterns::FIVE:         return WIN_SCORE;
        case Patterns::OPEN_FOUR:    return 100'000;
        case Patterns::CLOSED_FOUR:  return 10'000;
        case Patterns::OPEN_THREE:   return 1'000;
        case Patterns::CLOSED_THREE: return 100;
        case Patterns::OPEN_TWO:     return 100;
        case Patterns::CLOSED_TWO:   return 10;
        default:                     return 0;
    }
}

static int side_score(const Patterns::Counts& c) {
    int s = 0;
    for (int i = 0; i < Patterns::COUNT; ++i)
        s += c[i] * Eval::weight(static_cast<Patterns::Kind>(i));
    return s;
}

int Eval::score(const Board& b, Color c) {
    const Color opp = other(c);
    const auto own = b.count_patterns(c);
    const auto ops = b.count_patterns(opp);
    if (own[Patterns::FIVE] > 0) return WIN_SCORE;
    if (ops[Patterns::FIVE] > 0) return -WIN_SCORE;
    const int own_s = side_score(own);
    const int opp_s = side_score(ops);
    return own_s - (opp_s * 11) / 10;
}

} // namespace gomoku
