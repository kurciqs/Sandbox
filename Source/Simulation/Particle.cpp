#include "Particle.h"

void Particle::ApplyForce(glm::vec3 force) {
    if (std::isnan(force.x) || std::isnan(force.y) || std::isnan(force.z)) {
        print_error("Particle::ApplyForce: NaN detected\n", 0);
        return;
    }
    acc += force / mass;
}
