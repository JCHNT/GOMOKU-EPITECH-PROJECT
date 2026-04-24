#include "catch.hpp"
#include "engine/patterns.hpp"

using namespace gomoku;

static std::pair<uint32_t,uint32_t> make_line(const std::string& s) {
    uint32_t own = 0, opp = 0;
    for (size_t i = 0; i < s.size() && i < 20; ++i) {
        if (s[i] == 'X') own |= (1u << i);
        else if (s[i] == 'O') opp |= (1u << i);
    }
    return {own, opp};
}

TEST_CASE("detects FIVE in a row", "[patterns]") {
    auto [own, opp] = make_line("....XXXXX...........");
    auto counts = Patterns::count_line(own, opp);
    REQUIRE(counts[Patterns::FIVE] == 1);
}

TEST_CASE("detects OPEN_FOUR", "[patterns]") {
    auto [own, opp] = make_line(".....XXXX...........");
    auto counts = Patterns::count_line(own, opp);
    REQUIRE(counts[Patterns::OPEN_FOUR] == 1);
}

TEST_CASE("closed four: blocked one side", "[patterns]") {
    auto [own, opp] = make_line("....OXXXX...........");
    auto counts = Patterns::count_line(own, opp);
    REQUIRE(counts[Patterns::CLOSED_FOUR] == 1);
    REQUIRE(counts[Patterns::OPEN_FOUR] == 0);
}

TEST_CASE("closed four: gapped X_XXX", "[patterns]") {
    auto [own, opp] = make_line("....X.XXX...........");
    auto counts = Patterns::count_line(own, opp);
    REQUIRE(counts[Patterns::CLOSED_FOUR] == 1);
}

TEST_CASE("open three: _XXX_", "[patterns]") {
    auto [own, opp] = make_line(".....XXX............");
    auto counts = Patterns::count_line(own, opp);
    REQUIRE(counts[Patterns::OPEN_THREE] == 1);
}

TEST_CASE("open three (gapped): _X_XX_ and _XX_X_", "[patterns]") {
    {
        auto [own, opp] = make_line("....X.XX............");
        auto counts = Patterns::count_line(own, opp);
        REQUIRE(counts[Patterns::OPEN_THREE] == 1);
    }
    {
        auto [own, opp] = make_line("....XX.X............");
        auto counts = Patterns::count_line(own, opp);
        REQUIRE(counts[Patterns::OPEN_THREE] == 1);
    }
}

TEST_CASE("closed three: blocked _XXXO", "[patterns]") {
    auto [own, opp] = make_line(".....XXXO...........");
    auto counts = Patterns::count_line(own, opp);
    REQUIRE(counts[Patterns::CLOSED_THREE] == 1);
    REQUIRE(counts[Patterns::OPEN_THREE] == 0);
}

TEST_CASE("open two: _XX_", "[patterns]") {
    auto [own, opp] = make_line(".....XX.............");
    auto counts = Patterns::count_line(own, opp);
    REQUIRE(counts[Patterns::OPEN_TWO] == 1);
}

TEST_CASE("board edges count as blockers", "[patterns]") {
    auto [own, opp] = make_line("XXXXX...............");
    auto counts = Patterns::count_line(own, opp);
    REQUIRE(counts[Patterns::FIVE] == 1);
}

TEST_CASE("empty line produces zero counts", "[patterns]") {
    auto [own, opp] = make_line("....................");
    auto counts = Patterns::count_line(own, opp);
    for (int i = 0; i < Patterns::COUNT; ++i) REQUIRE(counts[i] == 0);
}
