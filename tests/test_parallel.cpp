#ifdef BONUS
#include "catch.hpp"
#include "board/board.hpp"
#include "engine/search.hpp"
#include "engine/time_mgr.hpp"
#include "engine/tt.hpp"
#include "parallel.hpp"

using namespace gomoku;

TEST_CASE("parallel search finds mate-in-1", "[parallel][bonus]") {
    Board b;
    b.make_move({5, 10}, BLACK);
    b.make_move({6, 10}, BLACK);
    b.make_move({7, 10}, BLACK);
    b.make_move({8, 10}, BLACK);
    TranspositionTable tt(1 << 16);
    TimeMgr tm; tm.start_turn(BOARD_CELLS - 4);
    ParallelSearch ps(tt, tm, 4);
    Move best = ps.go(b, BLACK, 3);
    REQUIRE((best == Move{4, 10} || best == Move{9, 10}));
}
#endif
