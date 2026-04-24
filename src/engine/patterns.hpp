#pragma once

#include "util/types.hpp"
#include <array>
#include <cstdint>

namespace gomoku {

class Patterns {
public:
    enum Kind : uint8_t {
        FIVE = 0,
        OPEN_FOUR,
        CLOSED_FOUR,
        OPEN_THREE,
        CLOSED_THREE,
        OPEN_TWO,
        CLOSED_TWO,
        COUNT
    };

    using Counts = std::array<uint16_t, COUNT>;

    // Count all pattern occurrences along a single 20-cell line.
    // own = bitmask of own stones (bit i => cell i), opp = opponent stones.
    // Cells out of line range (above bit 19) are treated as blockers (opponent).
    static Counts count_line(uint32_t own, uint32_t opp);
};

} // namespace gomoku
