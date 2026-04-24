#include "catch.hpp"
#include "board/board.hpp"
#include "engine/eval.hpp"

using namespace gomoku;

TEST_CASE("empty board evaluates to 0", "[eval]") {
    Board b;
    REQUIRE(Eval::score(b, BLACK) == 0);
}

TEST_CASE("FIVE evaluates to WIN_SCORE for the side of play", "[eval]") {
    Board b;
    for (int x = 5; x <= 9; ++x) b.make_move({(int8_t)x, 10}, BLACK);
    REQUIRE(Eval::score(b, BLACK) >= WIN_SCORE);
    REQUIRE(Eval::score(b, WHITE) <= -WIN_SCORE);
}

TEST_CASE("OPEN_FOUR for own side is positive and large", "[eval]") {
    Board b;
    for (int x = 5; x <= 8; ++x) b.make_move({(int8_t)x, 10}, BLACK);
    int s = Eval::score(b, BLACK);
    REQUIRE(s > 50'000);
    REQUIRE(s < WIN_SCORE);
}

TEST_CASE("sign flip: swapping colors flips sign of score", "[eval]") {
    Board a;
    a.make_move({10, 10}, BLACK);
    a.make_move({11, 10}, BLACK);
    a.make_move({12, 10}, BLACK);
    int sa = Eval::score(a, BLACK);

    Board b;
    b.make_move({10, 10}, WHITE);
    b.make_move({11, 10}, WHITE);
    b.make_move({12, 10}, WHITE);
    int sb = Eval::score(b, BLACK);

    // Defensive tilt breaks strict symmetry: opp threats weigh 1.1x.
    REQUIRE(sa > 0);
    REQUIRE(sb < 0);
    // Magnitude of opponent threat slightly larger due to tilt.
    REQUIRE(-sb >= sa);
}

TEST_CASE("perspective flip preserves sign for dominant side", "[eval]") {
    Board b;
    b.make_move({5, 5}, BLACK);
    b.make_move({6, 5}, BLACK);
    REQUIRE(Eval::score(b, BLACK) > 0);
    REQUIRE(Eval::score(b, WHITE) < 0);
}

TEST_CASE("defensive tilt: opponent threats weigh more than own", "[eval]") {
    Board b;
    b.make_move({3, 3}, BLACK);
    b.make_move({4, 3}, BLACK);
    b.make_move({5, 3}, BLACK);
    b.make_move({3, 10}, WHITE);
    b.make_move({4, 10}, WHITE);
    b.make_move({5, 10}, WHITE);
    REQUIRE(Eval::score(b, BLACK) < 0);
}
