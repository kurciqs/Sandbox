#ifndef SANDBOX_SDFGENERATOR_H
#define SANDBOX_SDFGENERATOR_H

#include "Simulation/Constraint.h"
#include "Simulation/RigidBody.h"
#include "tinyobjloader.h"
#include "glm/gtx/hash.hpp"
#include <set>
#include "tmd.h"
#define TO_VEC3(x) glm::vec3(x[0], x[1], x[2])

#define IS_OUTER(x) (x >= 0.0f && x != 9999.0f)

struct Triangle {
    glm::vec3 v1;
    glm::vec3 v2;
    glm::vec3 v3;
    glm::vec3 n;
};

struct Prism {
    // floor
    glm::vec3 p1;
    glm::vec3 p2;
    glm::vec3 p3;

    // ceiling
    glm::vec3 p4;
    glm::vec3 p5;
    glm::vec3 p6;

    glm::vec3 operator[] (int index) const {
        switch(index) {
            case 0:
                return p1;
            case 1:
                return p2;
            case 2:
                return p3;
            case 3:
                return p4;
            case 4:
                return p5;
            case 5:
                return p6;
            default:
                return p1;
        }
    }
};

struct IAABB{
    glm::ivec3 min;
    glm::ivec3 max;
};

namespace Generator {
    SDFData SDFCube(glm::vec3 p, glm::vec3 b, float step);

    SDFData SDFBall(glm::vec3 p, float s, float step);

    // TODO
    std::vector<RigidBody*> RigidBodiesFromOBJ(const std::string& path, int rigidBodyID, int firstParticleIndex, glm::vec3 offset, glm::vec3 color, glm::vec3 vel);
}
#endif //SANDBOX_SDFGENERATOR_H
