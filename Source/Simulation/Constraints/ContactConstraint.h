#ifndef SANDBOX_CONTACTCONSTRAINT_H
#define SANDBOX_CONTACTCONSTRAINT_H

#include "Simulation/Constraint.h"

class ContactConstraint : public Constraint {
public:
    ContactConstraint(Particle* p1, Particle* p2, float k);
    virtual ~ContactConstraint();

    void Project() override;
    void Draw(Renderer& renderer) override;
};


#endif //SANDBOX_CONTACTCONSTRAINT_H