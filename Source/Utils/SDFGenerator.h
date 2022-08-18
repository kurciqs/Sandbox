#ifndef SANDBOX_SDFGENERATOR_H
#define SANDBOX_SDFGENERATOR_H

#include "Simulation/Constraint.h"
#include "Simulation/RigidBody.h"

namespace SDFGenerator {
    SDFData Cube(glm::vec3 p, glm::vec3 b, float step);

    SDFData Ball(glm::vec3 p, float s, float step);

    // TODO
    std::vector<SDFData> FromOBJ(std::string path);
}
#endif //SANDBOX_SDFGENERATOR_H
