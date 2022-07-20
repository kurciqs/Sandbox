#ifndef SANDBOX_PARTICLESYSTEM_H
#define SANDBOX_PARTICLESYSTEM_H

#include <vector>
#include "Simulation/Particle.h"
#include "Simulation/Constraints/DistanceConstraint.h"
#include "Graphics/Renderer.h"

using ConstraintGroup = std::vector<Constraint*>;

enum ParticleSystemType {
    Testing
};

class ParticleSystem {
public:
    ParticleSystem(int numParticles, ParticleSystemType type);
    void Update(float dt);
    void Draw();
    void Clear();
    void Destroy();
private:
    std::vector<Particle*> m_particles;
    std::vector<ConstraintGroup> m_constraints;
    glm::vec3 m_globalForce{0.0f};
};


#endif //SANDBOX_PARTICLESYSTEM_H
