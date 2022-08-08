#ifndef SANDBOX_PARTICLESYSTEM_H
#define SANDBOX_PARTICLESYSTEM_H

#include <vector>
#include <unordered_set>
#include "Simulation/Particle.h"
#include "Simulation/Constraints/DistanceConstraint.h"
#include "Simulation/Constraints/ContactConstraint.h"
#include "Simulation/Constraints/BoxBoundaryConstraint.h"
#include "Simulation/Constraints/PositionConstraint.h"
#include "Simulation/Constraints/RigidShapeConstraint.h"
#include "Graphics/Renderer.h"

#define k_distance 0.1f
#define k_contact 1.0f
#define GRAVITY
#define PARTICLE_SLEEPING_EPSILON 0.0001f
#define RANDOM_COLOR glm::vec3(rand() % 255, rand() % 255, rand() % 255) / 255.0f
static glm::vec3 lowerBoundary(-10.0f);
static glm::vec3 upperBoundary(10.0f);
using ConstraintGroup = std::vector<Constraint*>;

enum ParticleSystemType {
    Testing
};

enum ConstraintGroupEnum {
    CONTACT = 0,
    STANDARD = 1,
    SHAPE = 2
};

class ParticleSystem {
public:
    ParticleSystem(int numParticles, ParticleSystemType type);

    void Update(float dt);
    void Draw(Renderer& renderer);
    void Clear();
    void Destroy();
    void AddParticle(glm::vec3 pos, glm::vec3 vel, glm::vec3 color, float r);
    void AddCube(glm::vec3 pos, int width, int height, int depth);
    void AddBall(glm::vec3 pos, float radius) ;
private:
    std::vector<RigidBody*> m_rigidBodies;
    std::vector<Particle*> m_particles;
    std::vector<ConstraintGroup> m_constraints;
    glm::vec3 m_globalForce{0.0f, -9.8f, 0.0f}; // gravity
};


#endif //SANDBOX_PARTICLESYSTEM_H
