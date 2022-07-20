#ifndef SANDBOX_DISTANCECONSTRAINT_H
#define SANDBOX_DISTANCECONSTRAINT_H

#include "Simulation/Constraint.h"
#include "Graphics/Renderer.h"

class DistanceConstraint final : public Constraint {
public:
    DistanceConstraint(Particle* p1, Particle* p2, float k, float d);
    void Project() final;
    void Draw() final;
    ~DistanceConstraint() final = default;
private:
    float m_restDistance; // rest distance
};


#endif //SANDBOX_DISTANCECONSTRAINT_H
