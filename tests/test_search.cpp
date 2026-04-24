#include "catch.hpp"
#include "board/board.hpp"
#include "engine/search.hpp"
#include "engine/time_mgr.hpp"
#include "engine/tt.hpp"
#include <chrono>

using namespace gomoku;

TEST_CASE("search finds mate-in-1 (own open four)", "[search]") {
    Board b;
    b.make_move({5, 10}, BLACK);
    b.make_move({6, 10}, BLACK);
    b.make_move({7, 10}, BLACK);
    b.make_move({8, 10}, BLACK);
    TranspositionTable tt(1 << 16);
    TimeMgr tm; tm.start_turn(BOARD_CELLS - 4);
    Search s(tt, tm);
    Move best = s.go(b, BLACK, 2);
    REQUIRE((best == Move{4, 10} || best == Move{9, 10}));
}

TEST_CASE("search blocks opponent mate-in-1", "[search]") {
    Board b;
    b.make_move({3, 5}, WHITE);
    b.make_move({4, 5}, WHITE);
    b.make_move({5, 5}, WHITE);
    b.make_move({6, 5}, WHITE);
    TranspositionTable tt(1 << 16);
    TimeMgr tm; tm.start_turn(BOARD_CELLS - 4);
    Search s(tt, tm);
    Move best = s.go(b, BLACK, 2);
    REQUIRE((best == Move{2, 5} || best == Move{7, 5}));
}

TEST_CASE("search returns a legal move on empty board", "[search]") {
    Board b;
    TranspositionTable tt(1 << 16);
    TimeMgr tm; tm.start_turn(BOARD_CELLS);
    Search s(tt, tm);
    Move best = s.go(b, BLACK, 2);
    REQUIRE(Board::in_bounds(best));
    REQUIRE(b.at(best.x, best.y) == EMPTY);
}

TEST_CASE("search respects time budget (returns before 1s with 100ms budget)", "[search]") {
    Board b;
    b.make_move({10, 10}, BLACK);
    TranspositionTable tt(1 << 16);
    TimeMgr tm;
    tm.set_info("timeout_turn", "100");
    tm.start_turn(BOARD_CELLS - 1);
    Search s(tt, tm);
    auto t0 = std::chrono::steady_clock::now();
    (void) s.go(b, WHITE, 20);
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - t0).count();
    REQUIRE(dt < 1000);
}
