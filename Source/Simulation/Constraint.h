#ifndef SANDBOX_CONSTRAINT_H
#define SANDBOX_CONSTRAINT_H

#include <vector>
#include "Particle.h"
#define SOLVER_ITERATIONS 5

enum ConstraintType {
    equality,
    inequality
};

class Constraint {
public:
    virtual void Project() = 0;
    // For debug purposes
    virtual void Draw() = 0;
    virtual ~Constraint() {}; // WARNING: might be leaky tisms
protected:
    float m_stiffness;
    ConstraintType m_type;
    std::vector<Particle*> m_particles;
};


#endif //SANDBOX_CONSTRAINT_H
