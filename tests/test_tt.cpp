#include "catch.hpp"
#include "engine/tt.hpp"

using namespace gomoku;

TEST_CASE("TT stores and probes by key", "[tt]") {
    TranspositionTable tt(1 << 16);
    TTEntry e{};
    e.key = 0xDEADBEEFCAFEBABEULL;
    e.depth = 5;
    e.score = 1234;
    e.flag = TT_EXACT;
    e.best = {3, 4};
    tt.store(e);
    TTEntry out;
    REQUIRE(tt.probe(e.key, out) == true);
    REQUIRE(out.depth == 5);
    REQUIRE(out.score == 1234);
    REQUIRE(out.flag == TT_EXACT);
    REQUIRE(out.best == Move{3, 4});
}

TEST_CASE("TT miss returns false", "[tt]") {
    TranspositionTable tt(1 << 16);
    TTEntry out;
    REQUIRE(tt.probe(0x1234, out) == false);
}

TEST_CASE("TT replacement policy prefers deeper entry", "[tt]") {
    TranspositionTable tt(4);
    TTEntry a{0x1111, 3, 100, TT_EXACT, {0,0}, 0};
    TTEntry b{0x1111, 8, 200, TT_EXACT, {1,1}, 0};
    tt.store(a);
    tt.store(b);
    TTEntry out;
    REQUIRE(tt.probe(0x1111, out) == true);
    REQUIRE(out.depth == 8);
}
