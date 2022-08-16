#ifndef SANDBOX_RIGIDBODY_H
#define SANDBOX_RIGIDBODY_H

#include <vector>
#include <map>
#include "Simulation/Particle.h"
#include "Graphics/Renderer.h"
#include "Simulation/Constraint.h"

struct SDFData {
    glm::vec3 grad;
    float mag;
};

struct RigidBody {
    explicit RigidBody(int id): ID(id) {};
    int ID{};

    void AddVertex(Particle* p, SDFData d, int particleIndex);
    std::vector<Particle*> particles;
    std::map<int, SDFData> sdfData;

    void RecalculateCOM();
    glm::vec3 centerOfMass{};
    float totalMass{};

    void CalculateOffsets(); // don't add particles after this one
    std::vector<glm::vec3> offsets; // map<int - m_begin, .. (r-s)

    void UpdateMatrix();
    glm::mat3 rotation{1.0f};

    SDFData GetSDFData(int index);
};

#endif //SANDBOX_RIGIDBODY_H