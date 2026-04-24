#ifdef BONUS
#include "catch.hpp"
#include "board/board.hpp"
#include "tss.hpp"

using namespace gomoku;

TEST_CASE("VCF finds immediate win via open four", "[tss][bonus]") {
    Board b;
    b.make_move({5, 10}, BLACK);
    b.make_move({6, 10}, BLACK);
    b.make_move({7, 10}, BLACK);
    b.make_move({8, 10}, BLACK);
    Move w;
    REQUIRE(TSS::find_vcf(b, BLACK, 4, w) == true);
    REQUIRE((w == Move{4, 10} || w == Move{9, 10}));
}

TEST_CASE("VCF returns false on quiet position", "[tss][bonus]") {
    Board b;
    b.make_move({10, 10}, BLACK);
    Move w;
    REQUIRE(TSS::find_vcf(b, BLACK, 6, w) == false);
}
#endif
