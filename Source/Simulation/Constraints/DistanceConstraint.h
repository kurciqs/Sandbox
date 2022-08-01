#ifndef SANDBOX_DISTANCECONSTRAINT_H
#define SANDBOX_DISTANCECONSTRAINT_H

#include "Simulation/Constraint.h"

class DistanceConstraint : public Constraint {
public:
    DistanceConstraint(Particle* p1, Particle* p2, float k, float d);
    virtual ~DistanceConstraint();

    void Project() override;
    void Draw(Renderer& renderer) override;
private:
    float m_restDistance; // rest distance
};


#endif //SANDBOX_DISTANCECONSTRAINT_H
