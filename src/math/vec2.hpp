#pragma once

#include <cmath>
#include <iostream>

class vec2 {
public:
    // Default constructor initializes x and y to 0.0f
    vec2() : x_(0.0f), y_(0.0f) {}
    vec2(double x, double y) : x_(x), y_(y) {}

    // getters for x and y
    double x() const { return x_; }
    double y() const { return y_; }

    //setters for x and y
    void set_x(double x) { x_ = x; }
    void set_y(double y) { y_ = y; }

    // operater overrides
    vec2& operator+=(const vec2& other) {
        x_ += other.x_;
        y_ += other.y_;
        return *this;
    }
    vec2& operator-=(const vec2& other) {
        x_ -= other.x_;
        y_ -= other.y_;
        return *this;
    }
    vec2& operator*=(double scalar) {
        x_ *= scalar;
        y_ *= scalar;
        return *this;
    }
    vec2& operator/=(double scalar) {
        x_ /= scalar;
        y_ /= scalar;
        return *this;
    }

    vec2& operator=(const vec2& other) {
        if (this == &other) { return *this; }
        x_ = other.x_;
        y_ = other.y_;
        return *this;
    }

    bool operator==(const vec2& other) const {
        const double kEpsilon = 1e-6;
        return std::abs(x_ - other.x_) < kEpsilon &&
               std::abs(y_ - other.y_) < kEpsilon;
    }

    double dot(const vec2& other) const {
        return x_ * other.x_ + y_ * other.y_;
    }

    double length() const {
        return std::sqrt(x_ * x_ + y_ * y_);
    }

    // b = a.normalized() returns a new vector that is the normalized version of a, without modifying a.
    vec2 normalized() const {
        const double len = length();
        if (len > 0.0) {
            return vec2(x_ / len, y_ / len);
        }
        return *this;
    }

    // a.normalize() modifies a to be its normalized version and returns a reference to a.
    vec2& normalize() {
        *this = normalized();
        return *this;
    }

    vec2& rotate(double angle_rad) {
        const double cos_theta = std::cos(angle_rad);
        const double sin_theta = std::sin(angle_rad);
        const double new_x = x_ * cos_theta - y_ * sin_theta;
        const double new_y = x_ * sin_theta + y_ * cos_theta;
        x_ = new_x;
        y_ = new_y;
        return *this;
    }

private:
    double x_;
    double y_;
};

inline vec2 operator+(vec2 lhs, const vec2& rhs) {
    lhs += rhs;
    return lhs;
}

inline vec2 operator-(vec2 lhs, const vec2& rhs) {
    lhs -= rhs;
    return lhs;
}

inline vec2 operator*(double scalar, vec2 lhs) {
    lhs *= scalar;
    return lhs;
}

inline vec2 operator*(vec2 lhs, double scalar) {
    lhs *= scalar;
    return lhs;
}

inline vec2 operator/(vec2 lhs, double scalar) {
    lhs /= scalar;
    return lhs;
}

// 讓 REQUIRE 失敗時能印出實際數值，而不是一堆看不懂的原始位元組。
inline std::ostream& operator<<(std::ostream& os, const vec2& v) {
    os << "(" << v.x() << ", " << v.y() << ")";
    return os;
}