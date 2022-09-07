#ifndef SANDBOX_SDFGENERATOR_H
#define SANDBOX_SDFGENERATOR_H

#include "Simulation/Constraint.h"
#include "Simulation/RigidBody.h"
#include "tinyobjloader.h"
#include "glm/gtx/hash.hpp"
#include <set>
#include "tmd.h"
#define TO_VEC3(x) glm::vec3(x[0], x[1], x[2])

struct Material {
    glm::vec3 diffuse;
};

namespace Generator {
    SDFData SDFCube(glm::vec3 p, glm::vec3 b, float step);
    SDFData SDFBall(glm::vec3 p, float s, float step);

    std::vector<RigidBody*> RigidBodiesFromOBJ(const std::string& path, int rigidBodyID, int firstParticleIndex, glm::vec3 offset, glm::vec3 color = glm::vec3(-1.0f), glm::vec3 vel = glm::vec3(0.0f));
}
#endif //SANDBOX_SDFGENERATOR_H
