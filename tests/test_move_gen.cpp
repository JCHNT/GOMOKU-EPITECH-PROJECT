#include "catch.hpp"
#include "board/board.hpp"
#include "engine/move_gen.hpp"
#include <algorithm>

using namespace gomoku;

TEST_CASE("empty board returns only the center", "[move_gen]") {
    Board b;
    auto moves = MoveGen::candidates(b);
    REQUIRE(moves.size() == 1);
    REQUIRE(moves[0] == Move{10, 10});
}

TEST_CASE("after one stone, candidates within dist 2 + non-occupied", "[move_gen]") {
    Board b;
    b.make_move({10, 10}, BLACK);
    auto moves = MoveGen::candidates(b);
    REQUIRE(moves.size() > 0);
    for (auto m : moves) {
        REQUIRE(Board::in_bounds(m));
        REQUIRE(b.at(m.x, m.y) == EMPTY);
        int dx = std::abs(m.x - 10);
        int dy = std::abs(m.y - 10);
        REQUIRE(std::max(dx, dy) <= 2);
        REQUIRE(!(dx == 0 && dy == 0));
    }
}

TEST_CASE("candidates are unique", "[move_gen]") {
    Board b;
    b.make_move({5, 5}, BLACK);
    b.make_move({6, 6}, WHITE);
    auto moves = MoveGen::candidates(b);
    std::sort(moves.begin(), moves.end(), [](Move a, Move c){
        return (a.y < c.y) || (a.y == c.y && a.x < c.x);
    });
    for (size_t i = 1; i < moves.size(); ++i)
        REQUIRE(!(moves[i-1] == moves[i]));
}

TEST_CASE("full board returns empty", "[move_gen]") {
    Board b;
    for (int y = 0; y < BOARD_SIZE; ++y)
        for (int x = 0; x < BOARD_SIZE; ++x)
            b.make_move({(int8_t)x, (int8_t)y}, ((x + y) % 2) ? BLACK : WHITE);
    auto moves = MoveGen::candidates(b);
    REQUIRE(moves.empty());
}

TEST_CASE("ordering puts winning move first", "[move_gen][ordering]") {
    Board b;
    b.make_move({5, 10}, BLACK);
    b.make_move({6, 10}, BLACK);
    b.make_move({7, 10}, BLACK);
    b.make_move({8, 10}, BLACK);
    auto ordered = MoveGen::ordered_candidates(b, BLACK);
    REQUIRE(ordered.size() > 0);
    bool first_is_winning = (ordered[0] == Move{4, 10}) || (ordered[0] == Move{9, 10});
    REQUIRE(first_is_winning);
}

TEST_CASE("ordering prioritises blocking opp FIVE", "[move_gen][ordering]") {
    Board b;
    b.make_move({3, 5}, WHITE);
    b.make_move({4, 5}, WHITE);
    b.make_move({5, 5}, WHITE);
    b.make_move({6, 5}, WHITE);
    auto ordered = MoveGen::ordered_candidates(b, BLACK);
    REQUIRE(ordered.size() > 0);
    bool first_is_block = (ordered[0] == Move{2, 5}) || (ordered[0] == Move{7, 5});
    REQUIRE(first_is_block);
}
