#ifndef SANDBOX_PARTICLE_H
#define SANDBOX_PARTICLE_H

#include "glm/vec3.hpp"
#include <cmath>
#include "Utils/Utils.h"

struct Particle {
    void ApplyForce(glm::vec3 force);
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 acc;
    glm::vec3 color;
    float radius;
    float mass;
    float invMass;
    bool fixed;
};


#endif //SANDBOX_PARTICLE_H
