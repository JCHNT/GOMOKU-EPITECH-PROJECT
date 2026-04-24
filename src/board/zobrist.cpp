#include "board/zobrist.hpp"
#include <random>

namespace gomoku {

Zobrist::Zobrist() {
    std::mt19937_64 rng(0xC0FFEE1234567890ULL);
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < BOARD_CELLS; ++i)
            keys_[c][i] = rng();
}

const Zobrist& Zobrist::instance() {
    static const Zobrist z;
    return z;
}

} // namespace gomoku
