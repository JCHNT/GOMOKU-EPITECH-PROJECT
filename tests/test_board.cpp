#include "catch.hpp"
#include "board/board.hpp"

using namespace gomoku;

TEST_CASE("fresh board is empty with zero hash", "[board]") {
    Board b;
    REQUIRE(b.hash() == 0);
    for (int y = 0; y < BOARD_SIZE; ++y)
        for (int x = 0; x < BOARD_SIZE; ++x)
            REQUIRE(b.at(x, y) == EMPTY);
}

TEST_CASE("place + undo returns to empty and zero hash", "[board]") {
    Board b;
    b.make_move({5, 5}, BLACK);
    REQUIRE(b.at(5, 5) == BLACK);
    REQUIRE(b.hash() != 0);
    b.undo_move();
    REQUIRE(b.at(5, 5) == EMPTY);
    REQUIRE(b.hash() == 0);
}

TEST_CASE("hash is xor of placed keys (order-independent)", "[board]") {
    Board a, b;
    a.make_move({1, 2}, BLACK);
    a.make_move({3, 4}, WHITE);
    b.make_move({3, 4}, WHITE);
    b.make_move({1, 2}, BLACK);
    REQUIRE(a.hash() == b.hash());
}

TEST_CASE("double-place rejects and leaves state unchanged", "[board]") {
    Board b;
    REQUIRE(b.make_move({5, 5}, BLACK) == true);
    REQUIRE(b.make_move({5, 5}, WHITE) == false);
    REQUIRE(b.at(5, 5) == BLACK);
}

TEST_CASE("out-of-bounds rejects", "[board]") {
    Board b;
    REQUIRE(b.make_move({-1, 0}, BLACK) == false);
    REQUIRE(b.make_move({BOARD_SIZE, 0}, BLACK) == false);
    REQUIRE(b.make_move({0, BOARD_SIZE}, BLACK) == false);
}

TEST_CASE("stone_count reflects move history", "[board]") {
    Board b;
    REQUIRE(b.stone_count() == 0);
    b.make_move({10, 10}, BLACK);
    b.make_move({11, 10}, WHITE);
    REQUIRE(b.stone_count() == 2);
    b.undo_move();
    REQUIRE(b.stone_count() == 1);
}
