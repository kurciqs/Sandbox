#ifndef SANDBOX_RIGIDCONTACTCONSTRAINT_H
#define SANDBOX_RIGIDCONTACTCONSTRAINT_H

#include "Simulation/Constraint.h"
#include "Simulation/RigidBody.h"

class RigidContactConstraint : public Constraint {
public:
    RigidContactConstraint(RigidBody *rb1, RigidBody *rb2, Particle* p1, Particle* p2, float k);
    ~RigidContactConstraint() override;

    void Project() override;
    void Draw(Renderer& renderer) override;
private:
    std::vector<RigidBody*> m_rigidBodies;
};


#endif //SANDBOX_RIGIDCONTACTCONSTRAINT_H
