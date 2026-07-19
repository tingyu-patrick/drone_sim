#include <catch2/catch_test_macros.hpp>

#include "../src/math/vec2.hpp"

TEST_CASE("addition works", "[basic]") {
    REQUIRE(1 + 1 == 2);
}

// ---- vec2：建構與存取 ----

TEST_CASE("default constructor gives zero vector", "[vec2]") {
    vec2 v;
    REQUIRE(v == vec2(0.0, 0.0));
}

TEST_CASE("constructor and getters", "[vec2]") {
    vec2 v(3.0, 4.0);
    REQUIRE(v.x() == 3.0);
    REQUIRE(v.y() == 4.0);
}

TEST_CASE("setters modify components", "[vec2]") {
    vec2 v;
    v.set_x(1.5);
    v.set_y(-2.5);
    REQUIRE(v == vec2(1.5, -2.5));
}

// ---- 算術運算 ----

TEST_CASE("operator+= accumulates in place", "[vec2]") {
    vec2 v(1.0, 2.0);
    v += vec2(3.0, 4.0);
    REQUIRE(v == vec2(4.0, 6.0));
}

TEST_CASE("operator+ does not modify operands", "[vec2]") {
    const vec2 a(1.0, 2.0);
    const vec2 b(3.0, 4.0);
    vec2 c = a + b;
    REQUIRE(c == vec2(4.0, 6.0));
    REQUIRE(a == vec2(1.0, 2.0));
    REQUIRE(b == vec2(3.0, 4.0));
}

TEST_CASE("operator-= and operator-", "[vec2]") {
    vec2 v(5.0, 7.0);
    v -= vec2(3.0, 4.0);
    REQUIRE(v == vec2(2.0, 3.0));

    vec2 a(5.0, 7.0);
    vec2 b(3.0, 4.0);
    REQUIRE((a - b) == vec2(2.0, 3.0));
}

TEST_CASE("scalar multiplication works both directions", "[vec2]") {
    vec2 v(2.0, 3.0);
    REQUIRE((v * 2.0) == vec2(4.0, 6.0));
    REQUIRE((2.0 * v) == vec2(4.0, 6.0));
}

TEST_CASE("operator/= and operator/ divide by scalar", "[vec2]") {
    vec2 v(4.0, 6.0);
    v /= 2.0;
    REQUIRE(v == vec2(2.0, 3.0));

    vec2 a(4.0, 6.0);
    REQUIRE((a / 2.0) == vec2(2.0, 3.0));
}

// ---- 向量運算 ----

TEST_CASE("dot product", "[vec2]") {
    vec2 v1(1.0, 2.0);
    vec2 v2(3.0, 4.0);
    REQUIRE(v1.dot(v2) == 11.0);
}

TEST_CASE("length of a 3-4-5 vector", "[vec2]") {
    vec2 v(3.0, 4.0);
    REQUIRE(v.length() == 5.0);
}

TEST_CASE("normalized returns unit vector, original unchanged", "[vec2]") {
    const vec2 v(3.0, 4.0);
    vec2 n = v.normalized();

    REQUIRE(n == vec2(0.6, 0.8));
    REQUIRE(v == vec2(3.0, 4.0));  // v 本身沒被動到
    REQUIRE(std::abs(n.length() - 1.0) < 1e-6);
}

TEST_CASE("normalize mutates the vector in place", "[vec2]") {
    vec2 v(3.0, 4.0);
    v.normalize();
    REQUIRE(v == vec2(0.6, 0.8));
}

TEST_CASE("normalizing the zero vector does not crash or produce NaN", "[vec2]") {
    vec2 v(0.0, 0.0);
    vec2 n = v.normalized();
    REQUIRE(n == vec2(0.0, 0.0));

    v.normalize();
    REQUIRE(v == vec2(0.0, 0.0));
}

// ---- 比較 ----

TEST_CASE("operator==", "[vec2]") {
    REQUIRE(vec2(1.0, 2.0) == vec2(1.0, 2.0));
}

TEST_CASE("operator== tolerates tiny floating point error", "[vec2]") {
    vec2 a(0.1 + 0.2, 1.0);  // 經典的浮點數誤差案例，0.1+0.2 != 0.3（精確比較）
    vec2 b(0.3, 1.0);
    REQUIRE(a == b);
}