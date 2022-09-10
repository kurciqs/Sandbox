#include "FluidConstraint.h"

FluidConstraint::FluidConstraint(std::vector<Particle*>& allParticles, const std::vector<int>& fluidPartilces, float k, float density) {
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);
    m_density = density;

    for (int i : fluidPartilces) {
        m_particles.push_back(allParticles[i]);
        m_neighbours.emplace_back(); // empty vector
        m_lambdas.push_back(0.0f);
    }
    m_allParticles = &allParticles;

    IncrementCounts();
}

FluidConstraint::~FluidConstraint() {
    DecrementCounts();
}

void FluidConstraint::Project() {
    for (int i = 0; i < m_particles.size(); i++) {
        Particle* p_i = m_particles[i];


        float density_i = 0.0f; // pi
        float denom = 0.0f;

        for (int j = 0; j < m_allParticles->size(); j++) {
            Particle* p_j = m_allParticles->at(j);

            glm::vec3 diff = p_i->cpos - p_j->cpos;
            float d2 = glm::dot(diff, diff);
            if (d2 < H2 /* otherwise negative (neighbour) */ ) {
                // found neighbour
                density_i += poly6(d2) * p_j->mass;
                m_neighbours[i].push_back(p_j);

                // TODO;
                glm::vec3 gr = grad(0, 0, 0);
            }
        }


        m_lambdas[i] = -((density_i / m_density) - 1.0f) / (denom + EPSILON_RELAX);
    }
}

void FluidConstraint::Draw(Renderer &renderer) {

}

float FluidConstraint::poly6(float r2) {
    if(r2 >= H2) return 0.0f;
    float term2 = (H2 - r2);
    return (315.0f / (64.0f * (float)M_PI * H9)) * (term2 * term2 * term2);
}

glm::vec3 FluidConstraint::spikyGrad(const glm::vec3 &r, float r2) {
    if(r2 >= H) return {};
    if(r2 == 0) return {};
    return -glm::normalize(r) * (45.0f / ((float)M_PI * H6)) * (H - r2) * (H - r2);
}

glm::vec3 FluidConstraint::grad(int i, int k, int j) {
    Particle* p_k = (*m_allParticles)[k];
    float denom = 1.0f / m_density;
    glm::vec3 grad(0.0f);

    //TODO:
    for (Particle* p_j : *m_allParticles) {

    }


    return glm::vec3();
}
