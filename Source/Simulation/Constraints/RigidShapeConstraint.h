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
//    void RecalculateCOM();
//    glm::vec3 m_centerOfMass{};
//    int ID{};
//    float m_totalMass{};
//    std::vector<glm::vec3> m_offsets; // map<int - m_begin, .. (r-s)
};


#endif //SANDBOX_RIGIDSHAPECONSTRAINT_H
