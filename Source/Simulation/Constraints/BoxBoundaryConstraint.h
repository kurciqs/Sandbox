#ifndef SANDBOX_BOXBOUNDARYCONSTRAINT_H
#define SANDBOX_BOXBOUNDARYCONSTRAINT_H

#include "Simulation/Constraint.h"

class BoxBoundaryConstraint : public Constraint {
public:
    BoxBoundaryConstraint(Particle* p1, glm::vec3 min, glm::vec3 max, float k);
    virtual ~BoxBoundaryConstraint();

    void Project() override;
    void Draw(Renderer& renderer) override;
private:
    glm::vec3 m_min{};
    glm::vec3 m_max{};
};


#endif //SANDBOX_BOXBOUNDARYCONSTRAINT_H
