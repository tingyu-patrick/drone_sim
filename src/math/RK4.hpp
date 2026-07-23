#pragma once

#include "vec2.hpp"
#include <functional>

inline vec2 integrate(std::function<vec2(double, vec2)> f, vec2 state, double t0, double dt) {
    vec2 k1 = f(t0, state);
    vec2 k2 = f(t0 + dt / 2.0, state + k1 * dt / 2.0);
    vec2 k3 = f(t0 + dt / 2.0, state + k2 * dt / 2.0);
    vec2 k4 = f(t0 + dt, state + k3 * dt);
    return state + (k1 + 2.0 * k2 + 2.0 * k3 + k4) * dt / 6.0;
}