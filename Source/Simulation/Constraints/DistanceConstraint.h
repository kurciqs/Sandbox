#ifndef SANDBOX_DISTANCECONSTRAINT_H
#define SANDBOX_DISTANCECONSTRAINT_H

#include "Simulation/Constraint.h"

class DistanceConstraint : public Constraint {
public:
    DistanceConstraint(Particle* p1, Particle* p2, float k, float d, float tearDistance = 0.0f);
    virtual ~DistanceConstraint();

    void Project() override;
    void Draw(Renderer& renderer) override;
private:
    float m_restDistance; // rest distance
    float m_tearDistance;
};


#endif //SANDBOX_DISTANCECONSTRAINT_H
