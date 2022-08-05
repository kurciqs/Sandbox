#ifndef SANDBOX_RIGIDBODY_H
#define SANDBOX_RIGIDBODY_H

#include <map>
#include "glm/vec3.hpp"
#include <glm/gtx/fast_square_root.hpp>
#include <vector>
#include "Simulation/Particle.h"

struct SDFData {
    glm::vec3 grad;
    float mag;
};

class RigidBody {
public:
    RigidBody(int begin, int end, const std::vector<Particle*>& particles);
    void RecalculateCOM(const std::vector<Particle*>& particles);
    float DistanceToCOM(glm::vec3 point, int index = -1);
    float GetRestConfigDistanceToCOM(int index);
private:
    glm::vec3 m_centerOfMass;
    std::vector<SDFData> m_sdfMap; // map<int - m_begin, ..
    std::vector<glm::vec3> m_offsets; // map<int - m_begin, ..
    int m_beginIndex, m_lastIndex;
};

#endif //SANDBOX_RIGIDBODY_H