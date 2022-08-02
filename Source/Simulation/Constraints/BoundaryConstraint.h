#ifndef SANDBOX_BOUNDARYCONSTRAINT_H
#define SANDBOX_BOUNDARYCONSTRAINT_H

#include "Simulation/Constraint.h"

class BoundaryConstraint : public Constraint {
public:
    BoundaryConstraint(Particle* p1, glm::vec3 min, glm::vec3 max, float k);
    virtual ~BoundaryConstraint();

    void Project() override;
    void Draw(Renderer& renderer) override;
private:
    glm::vec3 m_min{};
    glm::vec3 m_max{};
};


#endif //SANDBOX_BOUNDARYCONSTRAINT_H
