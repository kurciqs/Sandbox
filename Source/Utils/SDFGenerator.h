#ifndef SANDBOX_SDFGENERATOR_H
#define SANDBOX_SDFGENERATOR_H

#include "Simulation/Constraint.h"
#include "Simulation/RigidBody.h"
#include "tinyobjloader.h"

namespace Generator {
    SDFData SDFCube(glm::vec3 p, glm::vec3 b, float step);

    SDFData SDFBall(glm::vec3 p, float s, float step);

    // TODO
    RigidBody* RigidBodyFromOBJ(glm::vec3 offset, std::string path, int rigidBodyID);
}
#endif //SANDBOX_SDFGENERATOR_H
