#ifndef SANDBOX_RIGIDBODY_H
#define SANDBOX_RIGIDBODY_H

#include <vector>
#include "Simulation/Particle.h"
#include "Graphics/Renderer.h"

struct SDFData {
    glm::vec3 grad;
    float mag;
};

class RigidBody {
public:
    RigidBody(int begin, int end, const std::vector<Particle*>& particles); // use all particles from particles[being] to particles[end]
    void AddVertex(const std::vector<Particle*>& particles, int index); // push extra ones
    void RecalculateCOM(const std::vector<Particle*>& particles, bool particlesOrdered);

    friend class RigidShapeConstraint;
private:
    float m_totalMass{};
    std::vector<int> m_indices; // index in the main particle array
    glm::vec3 m_centerOfMass{};
    std::vector<SDFData> m_sdfMap; // map<int - m_begin, ..
    std::vector<glm::vec3> m_offsets; // map<int - m_begin, .. (r-s)
};

#endif //SANDBOX_RIGIDBODY_H