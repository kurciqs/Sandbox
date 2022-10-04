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
#define k_shape 0.1f

#define PARTICLE_SLEEPING_EPSILON 0.001f

static glm::vec3 lowerBoundary( -20.f);
static glm::vec3 upperBoundary(  20.f);
using ConstraintGroup = std::vector<Constraint*>;

enum ParticleSystemType {
    Testing,
    Pool,
    Fluid,
    Cloth
};

enum ConstraintGroupEnum {
    CONTACT,
    SHAPE,
    FLUID,
    STANDARD
};

class ParticleSystem {
public:
    ParticleSystem(int numParticles, ParticleSystemType type);

    void Update(float dt);
    void Draw(Renderer& renderer);
    void Clear();
    void Destroy();
    void SetGlobalForce(glm::vec3 g);

    void AddParticle(glm::vec3 pos, glm::vec3 vel, glm::vec3 color, float r);
    void AddObject(glm::vec3 offset, const std::string& fileToObj, float mass = 1.0f, bool fixed = false, float stiffness = 1.0f, glm::vec3 color = glm::vec3(-1.0f), glm::vec3 vel = glm::vec3(0.0f));

    void AddCube(glm::vec3 pos, glm::vec3 vel, int width, int height, int depth, glm::vec3 color, float mass = 1.0f, bool fixed = false, float stiffness = 1.0f);
    void AddBall(glm::vec3 center, glm::vec3 vel, float radius, glm::vec3 color, float mass = 1.0f, bool fixed = false, float stiffness = 1.0f);
    void AddTorus(glm::vec3 center, glm::vec3 vel, float innerRadius, float outerRadius, glm::vec3 color, float mass = 1.0f, bool fixed = false, float stiffness = 1.0f);
    void AddCylinder(glm::vec3 center, glm::vec3 vel, float height, float radius, glm::vec3 color, float mass = 1.0f, bool fixed = false, float stiffness = 1.0f);
    void AddCone(glm::vec3 center, glm::vec3 vel, float angle, float height, glm::vec3 color, float mass = 1.0f, bool fixed = false, float stiffness = 1.0f);

    void AddCloth(glm::vec3 center, glm::vec3 vel, float width, float height, float stiffness, glm::bvec4 cornersFixed, float tearDistance);
    int AddFluid(int numParticles, float spawnRadius, glm::vec3 offset, glm::vec3 color, float density, float viscosity);
    void EmitFluidParticle(int ID, glm::vec3 pos, glm::vec3 vel);
    int GetFluidAmount();
private:
    std::vector<Particle*> m_particles;
    std::vector<ConstraintGroup> m_constraints;

    std::vector<RigidBody*> m_rigidBodies;
    glm::vec3 m_globalForce{0.0f, -9.8f, 0.0f}; // gravity
};

#endif //SANDBOX_PARTICLESYSTEM_H