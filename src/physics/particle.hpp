#pragma once

#include "../math/vec2.hpp"

struct Particle {
    vec2 position;
    vec2 velocity;
};

inline void step_particle(Particle& p, double g, double dt) {
    p.velocity.set_y(p.velocity.y() + g * dt);

    p.position.set_x(p.position.x() + p.velocity.x() * dt);
    p.position.set_y(p.position.y() + p.velocity.y() * dt);
}

inline Particle simulate_free_fall(vec2 initial_position,
                                    vec2 initial_velocity,
                                    double g,
                                    double dt,
                                    int steps) {
    Particle p{initial_position, initial_velocity};
    for (int i = 0; i < steps; ++i) {
        step_particle(p, g, dt);
        // vec2 state(p.position.y(), p.velocity.y());
        // auto f = [g](double t, vec2 state) {
        //     double position = state.x();
        //     double velocity = state.y();
        //     double dposition_dt = velocity;
        //     double dvelocity_dt = g;
        //     return vec2(dposition_dt, dvelocity_dt);
        // };

        // vec2 new_state = integrate(f, state, 0.0, dt);
        // p.position.set_y(new_state.x());
        // p.velocity.set_y(new_state.y());

        // // std::cout << "Step " << i << ": Position = (" << p.position.x() << ", " << p.position.y() << "), "
        // //      << "Velocity = (" << p.velocity.x() << ", " << p.velocity.y() << ")" << std::endl;
    }
    return p;
}

inline double analytical_y(double y0, double vy0, double g, double t) {
    return y0 + vy0 * t + 0.5 * g * t * t;
}