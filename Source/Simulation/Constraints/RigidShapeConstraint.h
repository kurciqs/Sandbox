#ifndef SANDBOX_RIGIDSHAPECONSTRAINT_H
#define SANDBOX_RIGIDSHAPECONSTRAINT_H

#include "Simulation/Constraint.h"
#include "Simulation/RigidBody.h"


class RigidShapeConstraint : public Constraint {
public:
    RigidShapeConstraint(RigidBody* rb, float k);
    ~RigidShapeConstraint() override;

    void Project() override;
    void Draw(Renderer& renderer) override;
private:
    RigidBody* m_rb;
    std::vector<float> m_distances;
};


#endif //SANDBOX_RIGIDSHAPECONSTRAINT_H
