#include "engine/patterns.hpp"

namespace gomoku {

static constexpr int LINE_LEN = 20;

// Off-line cells treated as opponent (edge = blocker).
static inline bool is_own(uint32_t own, int i) {
    return i >= 0 && i < LINE_LEN && (own & (1u << i));
}
static inline bool is_opp(uint32_t opp, int i) {
    if (i < 0 || i >= LINE_LEN) return true;
    return opp & (1u << i);
}
static inline bool is_empty(uint32_t own, uint32_t opp, int i) {
    return !is_own(own, i) && !is_opp(opp, i);
}

static inline bool match_five(uint32_t own, int i) {
    if (i < 0 || i + 4 >= LINE_LEN) return false;
    uint32_t mask = 0b11111u << i;
    return (own & mask) == mask;
}

static inline bool match_open_four(uint32_t own, uint32_t opp, int i) {
    if (i < 1 || i + 4 > LINE_LEN) return false;
    if (!is_empty(own, opp, i - 1)) return false;
    for (int k = 0; k < 4; ++k) if (!is_own(own, i + k)) return false;
    if (!is_empty(own, opp, i + 4)) return false;
    // Also reject if this is start of FIVE (i-1 empty prevents that, but i+4 empty does too).
    return true;
}

static inline bool match_closed_four_contig(uint32_t own, uint32_t opp, int i) {
    if (i < 0 || i + 3 >= LINE_LEN) return false;
    for (int k = 0; k < 4; ++k) if (!is_own(own, i + k)) return false;
    bool left_blocked  = (i == 0) || is_opp(opp, i - 1);
    bool right_blocked = (i + 4 >= LINE_LEN) || is_opp(opp, i + 4);
    bool left_empty  = (i > 0) && is_empty(own, opp, i - 1);
    bool right_empty = (i + 4 < LINE_LEN) && is_empty(own, opp, i + 4);
    // Reject if it's actually FIVE (5 own in row) — handled by match_five
    if (is_own(own, i + 4)) return false;
    if (i > 0 && is_own(own, i - 1)) return false;
    if ((left_blocked && right_empty) || (left_empty && right_blocked)) return true;
    return false;
}

static inline bool match_gapped_four(uint32_t own, uint32_t opp, int i) {
    if (i < 0 || i + 4 >= LINE_LEN) return false;
    int own_count = 0, empty_count = 0;
    int empty_pos = -1;
    for (int k = 0; k < 5; ++k) {
        if (is_own(own, i + k)) ++own_count;
        else if (is_empty(own, opp, i + k)) { ++empty_count; empty_pos = i + k; }
        else return false;
    }
    if (own_count != 4 || empty_count != 1) return false;
    // Only count when empty is strictly inside (not at i or i+4) to avoid
    // overlap with contig closed four scanner.
    return empty_pos > i && empty_pos < i + 4;
}

static inline bool match_open_three(uint32_t own, uint32_t opp, int i) {
    // Contiguous: _XXX_ with strict empties on both ends.
    if (i >= 1 && i + 3 < LINE_LEN) {
        bool ok = is_empty(own, opp, i - 1)
               && is_own(own, i) && is_own(own, i + 1) && is_own(own, i + 2)
               && is_empty(own, opp, i + 3);
        if (ok) {
            if (i - 2 >= 0 && is_own(own, i - 2)) ok = false;
            if (i + 4 < LINE_LEN && is_own(own, i + 4)) ok = false;
        }
        if (ok) return true;
    }
    // Gapped: _X_XX_ or _XX_X_ in 6-window starting at i.
    if (i >= 0 && i + 5 < LINE_LEN) {
        if (is_empty(own, opp, i) && is_own(own, i+1) && is_empty(own, opp, i+2)
            && is_own(own, i+3) && is_own(own, i+4) && is_empty(own, opp, i+5))
            return true;
        if (is_empty(own, opp, i) && is_own(own, i+1) && is_own(own, i+2)
            && is_empty(own, opp, i+3) && is_own(own, i+4) && is_empty(own, opp, i+5))
            return true;
    }
    return false;
}

static inline bool match_closed_three(uint32_t own, uint32_t opp, int i) {
    if (i < 0 || i + 2 >= LINE_LEN) return false;
    if (!(is_own(own, i) && is_own(own, i+1) && is_own(own, i+2))) return false;
    bool left_blocked  = (i == 0) || is_opp(opp, i - 1);
    bool right_blocked = (i + 3 >= LINE_LEN) || is_opp(opp, i + 3);
    bool left_empty  = (i > 0) && is_empty(own, opp, i - 1);
    bool right_empty = (i + 3 < LINE_LEN) && is_empty(own, opp, i + 3);
    if ((left_blocked && right_empty) || (left_empty && right_blocked)) {
        if (i - 2 >= 0 && is_own(own, i - 2)) return false;
        if (i + 4 < LINE_LEN && is_own(own, i + 4)) return false;
        if (i - 1 >= 0 && is_own(own, i - 1)) return false;
        if (i + 3 < LINE_LEN && is_own(own, i + 3)) return false;
        return true;
    }
    return false;
}

static inline bool match_open_two(uint32_t own, uint32_t opp, int i) {
    if (i < 1 || i + 2 >= LINE_LEN) return false;
    if (!(is_empty(own, opp, i - 1) && is_own(own, i) && is_own(own, i + 1) && is_empty(own, opp, i + 2)))
        return false;
    if (i - 2 >= 0 && is_own(own, i - 2)) return false;
    if (i + 3 < LINE_LEN && is_own(own, i + 3)) return false;
    return true;
}

static inline bool match_closed_two(uint32_t own, uint32_t opp, int i) {
    if (i < 0 || i + 1 >= LINE_LEN) return false;
    if (!(is_own(own, i) && is_own(own, i + 1))) return false;
    bool left_blocked  = (i == 0) || is_opp(opp, i - 1);
    bool right_blocked = (i + 2 >= LINE_LEN) || is_opp(opp, i + 2);
    bool left_empty  = (i > 0) && is_empty(own, opp, i - 1);
    bool right_empty = (i + 2 < LINE_LEN) && is_empty(own, opp, i + 2);
    if ((left_blocked && right_empty) || (left_empty && right_blocked)) {
        if (i - 2 >= 0 && is_own(own, i - 2)) return false;
        if (i + 3 < LINE_LEN && is_own(own, i + 3)) return false;
        if (i - 1 >= 0 && is_own(own, i - 1)) return false;
        if (i + 2 < LINE_LEN && is_own(own, i + 2)) return false;
        return true;
    }
    return false;
}

Patterns::Counts Patterns::count_line(uint32_t own, uint32_t opp) {
    Counts c{};
    for (int i = 0; i < LINE_LEN; ++i) {
        if (match_five(own, i))         ++c[FIVE];
        if (match_open_four(own, opp, i)) ++c[OPEN_FOUR];
        if (match_closed_four_contig(own, opp, i)) ++c[CLOSED_FOUR];
        if (match_gapped_four(own, opp, i)) ++c[CLOSED_FOUR];
        if (match_open_three(own, opp, i))  ++c[OPEN_THREE];
        if (match_closed_three(own, opp, i))++c[CLOSED_THREE];
        if (match_open_two(own, opp, i))    ++c[OPEN_TWO];
        if (match_closed_two(own, opp, i))  ++c[CLOSED_TWO];
    }
    return c;
}

} // namespace gomoku
