#ifdef BONUS
#include "catch.hpp"
#include "book.hpp"

using namespace gomoku;

TEST_CASE("book returns center on empty history", "[book][bonus]") {
    Book bk;
    bk.load_string("10,10\n");
    std::vector<Move> history;
    Move out;
    REQUIRE(bk.lookup(history, out) == true);
    REQUIRE(out == Move{10, 10});
}

TEST_CASE("book matches after opponent played 11,10 -> reply 10,11", "[book][bonus]") {
    Book bk;
    bk.load_string("10,10 11,10 10,11\n");
    std::vector<Move> history = {{10, 10}, {11, 10}};
    Move out;
    REQUIRE(bk.lookup(history, out) == true);
    REQUIRE(out == Move{10, 11});
}

TEST_CASE("book misses when history doesn't match", "[book][bonus]") {
    Book bk;
    bk.load_string("10,10 11,10 10,11\n");
    std::vector<Move> history = {{0, 0}};
    Move out;
    REQUIRE(bk.lookup(history, out) == false);
}
#endif
