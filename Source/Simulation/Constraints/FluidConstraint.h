#ifndef SANDBOX_FLUIDCONSTRAINT_H
#define SANDBOX_FLUIDCONSTRAINT_H

#include "Simulation/Constraint.h"

#define H 2.0f
#define H2 4.0f
#define H6 64.0f
#define H9 512.0f

#define EPSILON_RELAX 0.01f

#define K_CORR 0.1f
#define N_CORR 4
#define MAG_Q_CORR 0.2f

#define S_SOLID 0.0f

class FluidConstraint : public Constraint {
public:
    FluidConstraint(std::vector<Particle*>& allParticles, const std::vector<int>& fluidPartilces, float k, float density);
    virtual ~FluidConstraint();

    void Project() override;
    void Draw(Renderer& renderer) override;
private:
    float poly6(float r2);
    glm::vec3 spikyGrad(const glm::vec3& r, float r2);
    glm::vec3 grad(int k, int j);

    std::vector<std::vector<int>> m_neighbours;
    std::vector<int> m_particleIndices;
    std::vector<float> m_lambdas;
    float m_density;
};


#endif //SANDBOX_FLUIDCONSTRAINT_H
