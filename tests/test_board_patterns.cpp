#include "catch.hpp"
#include "board/board.hpp"
#include "engine/patterns.hpp"

using namespace gomoku;

TEST_CASE("board counts FIVE horizontal", "[board][patterns]") {
    Board b;
    for (int x = 5; x <= 9; ++x) b.make_move({(int8_t)x, 10}, BLACK);
    auto black_counts = b.count_patterns(BLACK);
    REQUIRE(black_counts[Patterns::FIVE] == 1);
}

TEST_CASE("board counts FIVE vertical", "[board][patterns]") {
    Board b;
    for (int y = 3; y <= 7; ++y) b.make_move({10, (int8_t)y}, BLACK);
    REQUIRE(b.count_patterns(BLACK)[Patterns::FIVE] == 1);
}

TEST_CASE("board counts FIVE main diagonal", "[board][patterns]") {
    Board b;
    for (int k = 0; k < 5; ++k) b.make_move({(int8_t)(4 + k), (int8_t)(4 + k)}, WHITE);
    REQUIRE(b.count_patterns(WHITE)[Patterns::FIVE] == 1);
}

TEST_CASE("board counts FIVE anti-diagonal", "[board][patterns]") {
    Board b;
    for (int k = 0; k < 5; ++k) b.make_move({(int8_t)(10 + k), (int8_t)(14 - k)}, WHITE);
    REQUIRE(b.count_patterns(WHITE)[Patterns::FIVE] == 1);
}

TEST_CASE("empty board = zero counts", "[board][patterns]") {
    Board b;
    for (int i = 0; i < Patterns::COUNT; ++i) {
        REQUIRE(b.count_patterns(BLACK)[i] == 0);
        REQUIRE(b.count_patterns(WHITE)[i] == 0);
    }
}

TEST_CASE("has_five returns true exactly when a 5 exists", "[board][patterns]") {
    Board b;
    for (int x = 2; x <= 5; ++x) b.make_move({(int8_t)x, 8}, BLACK);
    REQUIRE(b.has_five(BLACK) == false);
    b.make_move({6, 8}, BLACK);
    REQUIRE(b.has_five(BLACK) == true);
    REQUIRE(b.has_five(WHITE) == false);
}
