#ifndef SANDBOX_RIGIDCONTACTCONSTRAINT_H
#define SANDBOX_RIGIDCONTACTCONSTRAINT_H

#include "Simulation/Constraint.h"
#include "Simulation/RigidBody.h"
#define EPSILON 0.001f

class RigidContactConstraint : public Constraint {
public:
    RigidContactConstraint(Particle* p1, Particle* p2, SDFData d1, SDFData d2, float k);
    ~RigidContactConstraint() override;

    void Project() override;
    void Draw(Renderer& renderer) override;
private:
    std::vector<SDFData> m_SDFData;
};


#endif //SANDBOX_RIGIDCONTACTCONSTRAINT_H
