#include "Particle.h"

void Particle::ApplyForce(glm::vec3 f) {
    force += f;
}

Particle::Particle(glm::vec3 position, glm::vec3 color_, float mass_, float radius_, int phase_, bool fixed_) :
color(color_), mass(mass_), radius(radius_), phase(phase_), pos(position), invMass(1.0f / mass), fixed(fixed_), cpos(pos)
{}
