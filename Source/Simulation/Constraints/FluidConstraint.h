#ifndef SANDBOX_FLUIDCONSTRAINT_H
#define SANDBOX_FLUIDCONSTRAINT_H

#include "Simulation/Constraint.h"

#define H 2.0f
#define H2 4.0f
#define H6 64.0f
#define H9 512.0f
#define EPSILON_RELAX 0.1f

class FluidConstraint : public Constraint {
public:
    FluidConstraint(std::vector<Particle*>& allParticles, const std::vector<int>& fluidPartilces, float k, float density);
    virtual ~FluidConstraint();

    void Project() override;
    void Draw(Renderer& renderer) override;
private:
    float poly6(float r2);
    glm::vec3 spikyGrad(const glm::vec3& r, float r2);
    glm::vec3 grad(int i, int k, int j);

    std::vector<Particle*>* m_allParticles;
    std::vector<std::vector<Particle*>> m_neighbours;
    std::vector<float> m_lambdas;
    float m_density;
};


#endif //SANDBOX_FLUIDCONSTRAINT_H
