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

#include "protocol/dispatcher.hpp"
#include <sstream>

TEST_CASE("dispatcher responds OK to START 20", "[dispatcher]") {
    std::istringstream in("START 20\nEND\n");
    std::ostringstream out;
    Dispatcher d(in, out);
    d.run();
    std::string s = out.str();
    REQUIRE(s.find("OK") != std::string::npos);
}

TEST_CASE("dispatcher responds ERROR to START 19", "[dispatcher]") {
    std::istringstream in("START 19\nEND\n");
    std::ostringstream out;
    Dispatcher d(in, out);
    d.run();
    REQUIRE(out.str().find("ERROR") != std::string::npos);
}

TEST_CASE("dispatcher answers ABOUT", "[dispatcher]") {
    std::istringstream in("ABOUT\nEND\n");
    std::ostringstream out;
    Dispatcher d(in, out);
    d.run();
    std::string s = out.str();
    REQUIRE(s.find("name=") != std::string::npos);
    REQUIRE(s.find("version=") != std::string::npos);
}

TEST_CASE("dispatcher plays a move on BEGIN (center)", "[dispatcher]") {
    std::istringstream in("START 20\nBEGIN\nEND\n");
    std::ostringstream out;
    Dispatcher d(in, out);
    d.run();
    std::string s = out.str();
    REQUIRE(s.find("10,10") != std::string::npos);
}

TEST_CASE("dispatcher plays after TURN", "[dispatcher]") {
    std::istringstream in("START 20\nTURN 10,10\nEND\n");
    std::ostringstream out;
    Dispatcher d(in, out);
    d.run();
    std::string s = out.str();
    REQUIRE(s.find_first_of("0123456789") != std::string::npos);
}

TEST_CASE("dispatcher UNKNOWN for unrecognised cmd", "[dispatcher]") {
    std::istringstream in("FOO\nEND\n");
    std::ostringstream out;
    Dispatcher d(in, out);
    d.run();
    REQUIRE(out.str().find("UNKNOWN") != std::string::npos);
}

TEST_CASE("dispatcher RESTART clears board", "[dispatcher]") {
    std::istringstream in("START 20\nTURN 10,10\nRESTART\nBEGIN\nEND\n");
    std::ostringstream out;
    Dispatcher d(in, out);
    d.run();
    std::string s = out.str();
    REQUIRE(s.find("OK") != std::string::npos);
    size_t ok = s.find("OK");
    REQUIRE(s.find("10,10", ok) != std::string::npos);
}
