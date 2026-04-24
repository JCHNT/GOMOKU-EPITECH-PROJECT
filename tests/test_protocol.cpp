#include "catch.hpp"
#include "protocol/parser.hpp"

using namespace gomoku;

TEST_CASE("parse START 20 returns Start{20}", "[protocol]") {
    auto cmd = Parser::parse("START 20");
    REQUIRE(cmd.kind == Parser::START);
    REQUIRE(cmd.size == 20);
}

TEST_CASE("parse START with missing size fails", "[protocol]") {
    auto cmd = Parser::parse("START");
    REQUIRE(cmd.kind == Parser::MALFORMED);
}

TEST_CASE("parse TURN x,y", "[protocol]") {
    auto cmd = Parser::parse("TURN 10,11");
    REQUIRE(cmd.kind == Parser::TURN);
    REQUIRE(cmd.x == 10);
    REQUIRE(cmd.y == 11);
}

TEST_CASE("parse BEGIN / END / ABOUT / RESTART / BOARD / DONE", "[protocol]") {
    REQUIRE(Parser::parse("BEGIN").kind == Parser::BEGIN);
    REQUIRE(Parser::parse("END").kind == Parser::END);
    REQUIRE(Parser::parse("ABOUT").kind == Parser::ABOUT);
    REQUIRE(Parser::parse("RESTART").kind == Parser::RESTART);
    REQUIRE(Parser::parse("BOARD").kind == Parser::BOARD_START);
    REQUIRE(Parser::parse("DONE").kind == Parser::DONE);
}

TEST_CASE("parse INFO key value", "[protocol]") {
    auto cmd = Parser::parse("INFO timeout_turn 5000");
    REQUIRE(cmd.kind == Parser::INFO);
    REQUIRE(cmd.info_key == "timeout_turn");
    REQUIRE(cmd.info_value == "5000");
}

TEST_CASE("parse board-config line x,y,who (inside BOARD block)", "[protocol]") {
    auto cmd = Parser::parse("3,4,1");
    REQUIRE(cmd.kind == Parser::BOARD_CELL);
    REQUIRE(cmd.x == 3);
    REQUIRE(cmd.y == 4);
    REQUIRE(cmd.who == 1);
}

TEST_CASE("parse unknown command", "[protocol]") {
    auto cmd = Parser::parse("HELLO");
    REQUIRE(cmd.kind == Parser::UNKNOWN);
    REQUIRE(cmd.raw == "HELLO");
}

TEST_CASE("parse tolerates trailing \\r", "[protocol]") {
    auto cmd = Parser::parse("BEGIN\r");
    REQUIRE(cmd.kind == Parser::BEGIN);
}
