#include "engine/move_gen.hpp"
#include "engine/eval.hpp"
#include <algorithm>

namespace gomoku {

static constexpr int RADIUS = 2;

std::vector<Move> MoveGen::candidates(const Board& b) {
    std::vector<Move> out;
    if (b.stone_count() == 0) {
        int c = BOARD_SIZE / 2;
        out.push_back({(int8_t)c, (int8_t)c});
        return out;
    }

    bool seen[BOARD_SIZE][BOARD_SIZE] = {};
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            if (b.at(x, y) == EMPTY) continue;
            for (int dy = -RADIUS; dy <= RADIUS; ++dy) {
                for (int dx = -RADIUS; dx <= RADIUS; ++dx) {
                    int nx = x + dx, ny = y + dy;
                    if (dx == 0 && dy == 0) continue;
                    if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE) continue;
                    if (b.at(nx, ny) != EMPTY) continue;
                    if (seen[ny][nx]) continue;
                    seen[ny][nx] = true;
                    out.push_back({(int8_t)nx, (int8_t)ny});
                }
            }
        }
    }
    return out;
}

static int move_score(Board& b, Move m, Color c) {
    b.make_move(m, c);
    int s = Eval::score(b, c);
    b.undo_move();
    return s;
}

std::vector<Move> MoveGen::ordered_candidates(Board& b, Color to_move) {
    auto moves = candidates(b);
    std::vector<std::pair<int, Move>> scored;
    scored.reserve(moves.size());
    for (auto m : moves) scored.push_back({move_score(b, m, to_move), m});
    std::sort(scored.begin(), scored.end(),
              [](const auto& a, const auto& c) { return a.first > c.first; });
    std::vector<Move> out;
    out.reserve(scored.size());
    for (auto& p : scored) out.push_back(p.second);
    return out;
}

} // namespace gomoku
