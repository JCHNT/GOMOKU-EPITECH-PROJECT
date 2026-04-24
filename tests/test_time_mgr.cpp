#include "catch.hpp"
#include "engine/time_mgr.hpp"
#include <thread>

using namespace gomoku;

TEST_CASE("default budget is ~4500 ms", "[time_mgr]") {
    TimeMgr t;
    t.start_turn(50);
    REQUIRE(t.budget_ms() >= 4000);
    REQUIRE(t.budget_ms() <= 5000);
}

TEST_CASE("INFO timeout_turn is respected", "[time_mgr]") {
    TimeMgr t;
    t.set_info("timeout_turn", "1000");
    t.start_turn(50);
    REQUIRE(t.budget_ms() <= 1000);
    REQUIRE(t.budget_ms() >= 700);
}

TEST_CASE("hard deadline triggers stop after budget", "[time_mgr]") {
    TimeMgr t;
    t.set_info("timeout_turn", "30");
    t.start_turn(50);
    REQUIRE(t.should_stop() == false);
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    REQUIRE(t.should_stop() == true);
}

TEST_CASE("unknown INFO keys are silently ignored", "[time_mgr]") {
    TimeMgr t;
    t.set_info("unknown_key", "whatever");
    t.set_info("max_memory", "70000000");
    t.start_turn(50);
    REQUIRE(t.budget_ms() > 0);
}
