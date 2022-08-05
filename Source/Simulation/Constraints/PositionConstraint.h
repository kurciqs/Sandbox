#ifndef SANDBOX_POSITIONCONSTRAINT_H
#define SANDBOX_POSITIONCONSTRAINT_H

#include "Simulation/Constraint.h"

class PositionConstraint : public Constraint {
public:
    PositionConstraint(Particle* p, glm::vec3 pos, float k);
    ~PositionConstraint() override;

    void Project() override;
    void Draw(Renderer& renderer) override;
private:
    glm::vec3 m_pos{};
};


#endif //SANDBOX_POSITIONCONSTRAINT_H
