#ifndef SANDBOX_PARTICLE_H
#define SANDBOX_PARTICLE_H

#include "glm/vec3.hpp"
#include <cmath>
#include "Utils/Utils.h"

struct Particle {
    void ApplyForce(glm::vec3 force);
    glm::vec3 pos{0.0f};
    glm::vec3 vel{0.0f};
    glm::vec3 acc{0.0f};
    glm::vec3 color{1.0f};
    float radius = 1.0f;
    float mass = 1.0f;
    float invMass = 1.0f;
    bool fixed = true;
    int phase = 0;
};


#endif //SANDBOX_PARTICLE_H
