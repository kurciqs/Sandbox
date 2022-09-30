#ifndef SANDBOX_FLUIDCONSTRAINT_H
#define SANDBOX_FLUIDCONSTRAINT_H

#include "Simulation/Constraint.h"

//#define H 3.5f
//#define H2 12.25f
//#define H6 1838.265625f
//#define H9 78815.6386719f

//#define H 5.f
//#define H6 15625.f
//#define H9 1953125.f

//#define H 4.f
//#define H6 4096.f
//#define H9 262144.f

//#define H 2.0f
//#define H2 4.0f
//#define H6 64.0f
//#define H9 512.0f

//#define H 0.1f
//#define H2 0.01f
//#define H6 1e-06f
//#define H9 1e-09f


#define EPSILON_RELAX 1e+05f

#define K_CORR 0.1f
#define N_CORR 4.0f
#define MAG_Q_CORR 0.3f

#define VORTICITY_COEF 0.1f

#define S_SOLID 0.1f


class FluidConstraint : public Constraint {
public:
    FluidConstraint(int ID, std::vector<Particle*>& allParticles, const std::vector<int>& fluidPartilces, glm::vec3 color, float k, float density, float viscosityMag);
    virtual ~FluidConstraint();

    void Project() override;
    void Draw(Renderer& renderer) override;
    void AddParticle(int index);
    int m_ID;
    float m_viscosityMag;
    glm::vec3 m_color;
private:
    float Density(int i /*global index*/, int k);
    float Constraint(int i, int k);
    float m_kernelRadius = 2.0f;
    float k_Poly6 = (315.0f / (64.0f * (float)M_PI));
    float k_SpikyGrad = (45.0f / ((float)M_PI));
    float poly6(const glm::vec3& r);
    glm::vec3 spikyGrad(const glm::vec3& r);
    glm::vec3 ConstraintGradient(int k, int i, int j);

    std::vector<std::vector<int>> m_neighbours;
    std::vector<int> m_particleIndices;
    std::unordered_map<int, float> m_lambdas;
    std::vector<glm::vec3> m_deltas;
    std::vector<Particle*>* m_allParticles;
    float m_density;
};


#endif //SANDBOX_FLUIDCONSTRAINT_H
