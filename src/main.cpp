#include <cmath>
#include "csv_logger.hpp"
#include "math/vec2.hpp"

struct particle{
    vec2 position;
    vec2 velocity;
};

const double g = -9.8;
double t = 0;

void update_particle(particle& p, double dt) {
    // Update velocity based on gravity
    p.velocity.set_y(p.velocity.y() + g * dt); // Assuming downward is negative y direction

    // Update position based on velocity
    p.position.set_x(p.position.x() + p.velocity.x() * dt);
    p.position.set_y(p.position.y() + p.velocity.y() * dt);
}

int main() {
    particle p1;
    p1.position = vec2(0.0, 0.0);
    p1.velocity = vec2(1.0, 0.0);

    std::vector<std::string> headers = {"Time", "Position X", "Position Y", "Velocity X", "Velocity Y"};
    CSVLogger logger("output//particle_simulation.csv", headers);
    
    for (t = 0; t < 3; t += 0.001) {
        update_particle(p1, 0.001);
        logger.log({t, p1.position.x(), p1.position.y(), p1.velocity.x(), p1.velocity.y()});
    }

    return 0;
}