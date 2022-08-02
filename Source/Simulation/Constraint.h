#ifndef SANDBOX_CONSTRAINT_H
#define SANDBOX_CONSTRAINT_H

#include <vector>
#include "Particle.h"
#include "Graphics/Renderer.h"
#define SOLVER_ITERATIONS 5
#include <glm/gtx/fast_square_root.hpp>

class Constraint {
public:
    virtual ~Constraint() {};

    virtual void Project() = 0;
    virtual void Draw(Renderer& renderer) = 0;
protected:
    float m_stiffness;
    std::vector<Particle*> m_particles;
};


#endif //SANDBOX_CONSTRAINT_H
