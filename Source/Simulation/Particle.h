#ifndef SANDBOX_PARTICLE_H
#define SANDBOX_PARTICLE_H

#include "glm/vec3.hpp"
#include <cmath>
#include "Utils/Utils.h"
#define SOR_COEF 1.0f

struct Particle {
    void ApplyForce(glm::vec3 f);
    Particle(glm::vec3 position, glm::vec3 color_, float mass_ = 1.0f, float radius_ = 0.5f, int phase_ = 0, bool fixed_ = false);
    ~Particle() = default;
    glm::vec3 pos{0.0f};
    glm::vec3 cpos{0.0f};
    glm::vec3 vel{0.0f};
    glm::vec3 color{1.0f};
    glm::vec3 force{0.0f};
    float radius;
    float mass;
    float invMass;
    bool fixed;
    int phase;
    int num_constraints;
    int rigidBodyID;
    glm::mat3 R = glm::mat3(1.0f);
};


#endif //SANDBOX_PARTICLE_H
