#include "catch.hpp"
#include "board/zobrist.hpp"

using namespace gomoku;

TEST_CASE("zobrist keys are stable across calls", "[zobrist]") {
    const auto& k1 = Zobrist::instance();
    const auto& k2 = Zobrist::instance();
    REQUIRE(&k1 == &k2);
    REQUIRE(k1.key(BLACK, 0, 0) == k2.key(BLACK, 0, 0));
}

TEST_CASE("zobrist keys differ per (color,cell)", "[zobrist]") {
    const auto& z = Zobrist::instance();
    REQUIRE(z.key(BLACK, 0, 0) != z.key(WHITE, 0, 0));
    REQUIRE(z.key(BLACK, 0, 0) != z.key(BLACK, 1, 0));
    REQUIRE(z.key(BLACK, 5, 5) != 0);
}

TEST_CASE("zobrist XOR round-trip returns to zero", "[zobrist]") {
    const auto& z = Zobrist::instance();
    uint64_t h = 0;
    h ^= z.key(BLACK, 3, 4);
    h ^= z.key(WHITE, 7, 8);
    h ^= z.key(BLACK, 3, 4);
    h ^= z.key(WHITE, 7, 8);
    REQUIRE(h == 0);
}
