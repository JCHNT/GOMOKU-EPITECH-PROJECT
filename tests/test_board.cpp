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

TEST_CASE("line bitboards reflect placed stones (horizontal)", "[board][lines]") {
    Board b;
    b.make_move({3, 7}, BLACK);
    b.make_move({5, 7}, BLACK);
    uint32_t row = b.line_mask(BLACK, Board::LINE_ROW, 7);
    REQUIRE((row & (1u << 3)) != 0);
    REQUIRE((row & (1u << 5)) != 0);
    REQUIRE((row & (1u << 4)) == 0);
}

TEST_CASE("line bitboards reflect placed stones (vertical)", "[board][lines]") {
    Board b;
    b.make_move({10, 2}, WHITE);
    b.make_move({10, 9}, WHITE);
    uint32_t col = b.line_mask(WHITE, Board::LINE_COL, 10);
    REQUIRE((col & (1u << 2)) != 0);
    REQUIRE((col & (1u << 9)) != 0);
}

TEST_CASE("line bitboards clear on undo", "[board][lines]") {
    Board b;
    b.make_move({4, 4}, BLACK);
    REQUIRE(b.line_mask(BLACK, Board::LINE_ROW, 4) != 0);
    b.undo_move();
    REQUIRE(b.line_mask(BLACK, Board::LINE_ROW, 4) == 0);
}

TEST_CASE("diagonals track stones (main diagonal, x - y const)", "[board][lines]") {
    Board b;
    b.make_move({5, 5}, BLACK);
    b.make_move({7, 7}, BLACK);
    uint32_t diag = b.line_mask(BLACK, Board::LINE_DIAG, 19);
    REQUIRE((diag & (1u << 5)) != 0);
    REQUIRE((diag & (1u << 7)) != 0);
}

TEST_CASE("anti-diagonals track stones (x + y const)", "[board][lines]") {
    Board b;
    b.make_move({3, 7}, WHITE);
    b.make_move({6, 4}, WHITE);
    uint32_t anti = b.line_mask(WHITE, Board::LINE_ANTI, 10);
    REQUIRE((anti & (1u << 3)) != 0);
    REQUIRE((anti & (1u << 6)) != 0);
}

TEST_CASE("line_pos round-trips: single stone produces exactly one bit on each line", "[board][lines]") {
    using LK = Board::LineKind;
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            for (int k = 0; k < 4; ++k) {
                auto kind = static_cast<LK>(k);
                int idx = Board::line_index(kind, x, y);
                int pos = Board::line_pos(kind, x, y);
                Board b;
                b.make_move({(int8_t)x, (int8_t)y}, BLACK);
                uint32_t mask = b.line_mask(BLACK, kind, idx);
                INFO("x=" << x << " y=" << y << " kind=" << k << " idx=" << idx << " pos=" << pos);
                REQUIRE((mask & (1u << pos)) != 0);
                REQUIRE(__builtin_popcount(mask) == 1);
            }
        }
    }
}
