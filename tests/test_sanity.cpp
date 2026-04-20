#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("catch2 is wired up", "[sanity]") {
    REQUIRE(1 + 1 == 2);
}
