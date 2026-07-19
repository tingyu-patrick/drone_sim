#include <catch2/catch_test_macros.hpp>

TEST_CASE("addition works", "[basic]") {
    REQUIRE(1 + 1 == 2);
}

TEST_CASE("this one is intentionally wrong", "[basic]") {
    REQUIRE(1 + 1 == 3);
}