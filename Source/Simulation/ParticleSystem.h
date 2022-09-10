#ifndef SANDBOX_PARTICLESYSTEM_H
#define SANDBOX_PARTICLESYSTEM_H

#include <vector>
#include "Simulation/Particle.h"
#include "Simulation/Constraints/DistanceConstraint.h"
#include "Simulation/Constraints/ContactConstraint.h"
#include "Simulation/Constraints/BoxBoundaryConstraint.h"
#include "Simulation/Constraints/PositionConstraint.h"
#include "Simulation/Constraints/RigidShapeConstraint.h"
#include "Simulation/Constraints/RigidContactConstraint.h"
#include "Simulation/Constraints/FluidConstraint.h"
#include "Graphics/Renderer.h"
#include "Utils/SDFGenerator.h"

#define RANDOM_POS_IN_BOUNDARIES glm::vec3(rand() % int(fabsf(upperBoundary.x)) - rand() % int(fabsf(lowerBoundary.x)), \
                                           rand() % int(fabsf(upperBoundary.y)) - rand() % int(fabsf(lowerBoundary.y)), \
                                           rand() % int(fabsf(upperBoundary.z)) - rand() % int(fabsf(lowerBoundary.z)))

#define k_distance 0.1f
#define k_contact 1.0f
#define k_shape 1.0f

#define GRAVITY
#define PARTICLE_SLEEPING_EPSILON 0.001f

static glm::vec3 lowerBoundary( -5.0f);
static glm::vec3 upperBoundary(  5.0f);
using ConstraintGroup = std::vector<Constraint*>;

enum ParticleSystemType {
    Testing,
    Pool,
    Fluid
};

enum ConstraintGroupEnum {
    CONTACT,
    SHAPE,
    STANDARD
};

class ParticleSystem {
public:
    ParticleSystem(int numParticles, ParticleSystemType type);

    void Update(float dt);
    void Draw(Renderer& renderer);
    void Clear();
    void Destroy();

    void AddParticle(glm::vec3 pos, glm::vec3 vel, glm::vec3 color, float r);
    void AddObject(glm::vec3 offset, const std::string& fileToObj, glm::vec3 color = glm::vec3(-1.0f), glm::vec3 vel = glm::vec3(0.0f));

    void AddCube(glm::vec3 pos, glm::vec3 vel, int width, int height, int depth, glm::vec3 color);
    void AddBall(glm::vec3 center, glm::vec3 vel, float radius, glm::vec3 color);
    void AddTorus(glm::vec3 center, glm::vec3 vel, float innerRadius, float outerRadius, glm::vec3 color);
    void AddCylinder(glm::vec3 center, glm::vec3 vel, float height, float radius, glm::vec3 color);
    void AddCone(glm::vec3 center, glm::vec3 vel, float angle, float height, glm::vec3 color);
private:
    std::vector<Particle*> m_particles;
    std::vector<ConstraintGroup> m_constraints;

    std::vector<RigidBody*> m_rigidBodies;

    glm::vec3 m_globalForce{0.0f, -9.8f, 0.0f}; // gravity
};

#endif //SANDBOX_PARTICLESYSTEM_H