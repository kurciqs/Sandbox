#include "FluidConstraint.h"

FluidConstraint::FluidConstraint(int ID, std::vector<Particle*>& allParticles, const std::vector<int>& fluidPartilces, glm::vec3 color, float k, float density, float viscosityMag) {
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);
    m_density = density;
    m_viscosityMag = viscosityMag;
    m_ID = ID;

    m_color = color;
    m_allParticles = &allParticles;

    for (int i : fluidPartilces) {
        m_particleIndices.push_back(i);
        Particle *p_i = m_allParticles->at(i);
        p_i->color = m_color;
        p_i->num_constraints++;
    }
}

FluidConstraint::~FluidConstraint() {
    for (int i : m_particleIndices) {
        m_allParticles->at(i)->num_constraints--;
    }
}

void FluidConstraint::Project() {
    m_neighbours.clear();
    m_lambdas.clear();
    m_deltas.clear();
    for (int i = 0; i < m_allParticles->size(); i++) {
        m_neighbours.emplace_back();
        m_lambdas.emplace_back(0.0f);
        m_deltas.emplace_back(0.0f);
        m_neighbours[i].clear();
        Particle *p_i = m_allParticles->at(i);
        for (int j = 0; j < m_allParticles->size(); j++) {
            Particle *p_j = m_allParticles->at(j);
            glm::vec3 diff = p_i->cpos - p_j->cpos;
            float d2 = glm::dot(diff, diff);
            if (d2 < m_kernelRadius * m_kernelRadius) {
                m_neighbours[i].push_back(j);
            }
        }
    }

    for (int i = 0; i < m_allParticles->size(); i++) {
        const float numerator = Constraint(i);
        float denom = 0.0f;

        for (const int j: m_neighbours[i]) {
            const glm::vec3 grad = ConstraintGradient(i, j);

            denom += (1.0f / m_allParticles->at(j)->mass) * glm::dot(grad, grad);
        }

        denom += EPSILON_RELAX;

        m_lambdas[i] = -numerator / denom;
    }

    for (int k = 0; k < m_particleIndices.size(); k++) {
        int i = m_particleIndices[k];
        Particle *p_i = m_allParticles->at(i);
        const float corr_w = poly6(MAG_Q_CORR * m_kernelRadius * glm::vec3(1.0f, 0.0f, 0.0f));

        glm::vec3 sum(0.0f);
        for (const int j: m_neighbours[i]) {
            Particle *p_j = m_allParticles->at(j);

            const float kernel_val = poly6(p_i->cpos - p_j->cpos);
            const float ratio = kernel_val / corr_w;
            const float corr_coeff = -K_CORR * glm::pow(ratio, N_CORR);
            const float coeff = p_j->mass * (m_lambdas[i] + m_lambdas[j] + corr_coeff);

            sum += coeff * spikyGrad(p_i->cpos - p_j->cpos);
        }

        m_deltas[k] = (1.0f / p_i->mass) * (1.0f / m_density) * sum;
    }

    for (int k = 0; k < m_particleIndices.size(); k++) {
        int i = m_particleIndices[k];
        Particle *p_i = m_allParticles->at(i);

        p_i->cpos += m_deltas[k] * m_stiffness;
    }

    std::vector<float> densities;
    for (int i = 0; i < m_allParticles->size(); i++) {
        densities.push_back(Density(i));
    }

    for (int i = 0; i < m_allParticles->size(); i++) {
        Particle* p_i = m_allParticles->at(i);
        glm::vec3 omega(0.0f);

        const float density_i = densities[i];

        for (int k = 0; k < m_particleIndices.size(); k++) {
            int j = m_particleIndices[k];
            Particle* p_j = m_allParticles->at(j);
            const glm::vec3 diff = p_i->cpos - p_j->cpos;
            // only if neighbour
            if (glm::dot(diff, diff) < m_kernelRadius * m_kernelRadius) {
                const float density_j = densities[j];

                const glm::vec3 gradW = spikyGrad(diff);

                const glm::vec3 v_i = p_i->cpos - p_i->pos;
                const glm::vec3 v_j = p_j->cpos - p_j->pos;
                omega -= (p_j->mass / density_i) * glm::cross(v_i - v_j, gradW);
            }
        }

    }



    for (int k = 0; k < m_particleIndices.size(); k++) {
        int i = m_particleIndices[k];
        Particle *p_i = m_allParticles->at(i);

        glm::vec3 viscosity(0.0f);
        for (const int j : m_neighbours[i]) {
            Particle *p_j = m_allParticles->at(j);
            const float kernel_val = poly6(p_i->cpos - p_j->cpos);
            const glm::vec3 r_vel = p_j->vel - p_i->vel;

            viscosity += (p_i->mass / densities[j]) * kernel_val * r_vel;
        }

        p_i->viscosity = m_viscosityMag * viscosity;
    }


    //TODO: vorticity confinement
}

void FluidConstraint::Draw(Renderer &renderer) {
    for (int k = 0; k < m_particleIndices.size(); k++) {
        int i = m_particleIndices[k];
        Particle *p_i = m_allParticles->at(i);
        glm::vec3 delta = m_stiffness * m_deltas[k];
        renderer.DrawLine(p_i->cpos, p_i->cpos + delta, glm::vec3(1.0f));
    }
}

float FluidConstraint::poly6(const glm::vec3& r) {
    float r2 = glm::dot(r, r);

    if (r2 > m_kernelRadius2) {
        return 0.0f;
    }
    const float diff = m_kernelRadius2 - r2;
    const float diff3 = diff * diff * diff;

    return (k_Poly6 / m_kernelRadius9) * diff3;
}

glm::vec3 FluidConstraint::spikyGrad(const glm::vec3 &r) {
    float rlen = glm::fastLength(r);
    if (rlen > m_kernelRadius) {
        return glm::vec3(0.0f);
    }

    const float diff = m_kernelRadius - rlen;
    const float diff2 = diff * diff;

    return -r * (k_SpikyGrad / (m_kernelRadius6 * glm::max(rlen, 1e-24f))) * diff2;
}

glm::vec3 FluidConstraint::ConstraintGradient(int i, int j) {
    Particle* p_i = m_allParticles->at(i);

    if (i == j) {
        glm::vec3 sum(0.0f);
        for (int n: m_neighbours[i]) {
            Particle* p_n = m_allParticles->at(n);
            sum += p_n->mass * (p_n->phase == Phase::Solid ? S_SOLID : 1.0f) * spikyGrad(p_i->cpos - p_n->cpos);
        }
        return sum / m_density;
    }
    else {
        Particle* p_j = m_allParticles->at(j);
        return -p_j->mass * spikyGrad(p_i->cpos - p_j->cpos) / m_density;
    }
}

void FluidConstraint::AddParticle(int index) {
    m_particleIndices.push_back(index);
    m_allParticles->at(index)->num_constraints++;
    m_allParticles->at(index)->color = m_color;
}

float FluidConstraint::Density(int i) {
    Particle *p_i = m_allParticles->at(i);
    float density = 0.0f;
    for (const int n : m_neighbours[i]) {
        Particle* p_n = m_allParticles->at(n);
        density += p_n->mass * poly6(p_i->cpos - p_n->cpos);
    }
    return density;
}

float FluidConstraint::Constraint(int i) {
    const float density = Density(i);
    return (density / m_density) - 1.0f;
//    return density - m_density;
}
