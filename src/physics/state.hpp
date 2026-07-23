#pragma once

#include "../math/vec2.hpp"

struct State {
    vec2 position;
    vec2 velocity;
    double angle;
    double omega;
};

inline State operator+(const State& a, const State& b) {
    return State{a.position + b.position, a.velocity + b.velocity
        , a.angle + b.angle, a.omega + b.omega};
}

inline State operator*(const State& s, double scalar) {
    return State{s.position * scalar, s.velocity * scalar
        , s.angle * scalar, s.omega * scalar};
}

inline State operator*(double scalar, const State& s) {
    return s * scalar;
}