#include "FluidConstraint.h"

FluidConstraint::FluidConstraint(int ID, std::vector<Particle*>& allParticles, const std::vector<int>& fluidPartilces, glm::vec3 color, float k, float density, float viscosityMag) {
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);
    m_density = density;
    m_viscosityMag = viscosityMag;
    m_ID = ID;

    m_color = color;
    m_allParticles = &allParticles;
    for (int i = 0; i < m_allParticles->size(); i++) {
//    for (int i : fluidPartilces) {
        m_particleIndices.push_back(i);
        m_allParticles->at(i)->num_constraints++;
        m_allParticles->at(i)->color = m_color;
        m_allParticles->at(i)->fluidID = m_ID;
        m_neighbours.insert( {i, {}} ); // empty vector
        m_lambdas.emplace(i, 0.0f);
        m_deltas.emplace_back(0.0f);
    }
}

FluidConstraint::~FluidConstraint() {
    for (int i : m_particleIndices) {
        m_allParticles->at(i)->num_constraints--;
    }
}

void FluidConstraint::Project() {
    for (int k = 0; k < m_particleIndices.size(); k++) {
        m_neighbours[k].clear();
        int i = m_particleIndices[k];
        Particle *p_i = m_allParticles->at(i);
        for (int j = 0; j < m_allParticles->size(); j++) {
            Particle *p_j = m_allParticles->at(j);
            glm::vec3 diff = p_i->cpos - p_j->cpos;
            float d2 = glm::dot(diff, diff);
            if (d2 < m_kernelRadius * m_kernelRadius) {
                m_neighbours[k].push_back(j);
            }
        }
    }

    m_lambdas.clear();
    float avgRho = 0.0f;

    for (int k = 0; k < m_particleIndices.size(); k++) {
        int i = m_particleIndices[k];

        const float numerator = Constraint(i, k);
        float denom = 0.0f;
        for (const int j : m_neighbours[k]) {
            const glm::vec3 grad = ConstraintGradient(k, i, j);

            denom += (1.0f / m_allParticles->at(j)->mass) * glm::dot(grad, grad);
        }

        denom += EPSILON_RELAX;

        m_lambdas.insert_or_assign(i, -numerator / denom);
    }
    // TODO: make non fluid particles behave normally
    for (int k = 0; k < m_particleIndices.size(); k++) {
        int i = m_particleIndices[k];
        Particle *p_i = m_allParticles->at(i);
        if (p_i->fluidID != m_ID)
            continue;
        const float corr_w = poly6(MAG_Q_CORR * m_kernelRadius * glm::vec3(1.0f, 0.0f, 0.0f));

        std::vector<glm::vec3> buffer;
        for (const int j : m_neighbours[k]) {
            Particle* p_j = m_allParticles->at(j);

            const float kernel_val = poly6(p_i->cpos - p_j->cpos);
            const float ratio = kernel_val / corr_w;
            const float corr_coeff = -K_CORR * glm::pow(ratio, N_CORR);
            const float coeff = p_j->mass * (m_lambdas[i] + m_lambdas[j] + corr_coeff);

            buffer.push_back(coeff * spikyGrad(p_i->cpos - p_j->cpos));
        }

        glm::vec3 sum(0.0f);
        for (const glm::vec3 b : buffer) {
            sum += b;
        }

        m_deltas[i] = (1.0f / p_i->mass) * (1.0f / m_density) * sum;
    }

    for (int k = 0; k < m_particleIndices.size(); k++) {
        int i = m_particleIndices[k];
        Particle *p_i = m_allParticles->at(i);

        p_i->cpos += m_deltas[k] * m_stiffness;
    }
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
    const float h2 = m_kernelRadius * m_kernelRadius;
    float r2 = glm::dot(r, r);

    if (r2 > h2) {
        return 0.0f;
    }

    const float h4 = h2 * h2;
    const float h9 = h4 * h4 * m_kernelRadius;
    const float diff = h2 - r2;
    const float diff3 = diff * diff * diff;

    return (k_Poly6 / h9) * diff3;
}

glm::vec3 FluidConstraint::spikyGrad(const glm::vec3 &r) {
    float rlen = glm::fastLength(r);
    if (rlen > m_kernelRadius) {
        return glm::vec3(0.0f);
    }

    const float h2 = m_kernelRadius * m_kernelRadius;
    const float h6 = h2 * h2 * h2;
    const float diff = m_kernelRadius - rlen;
    const float diff2 = diff * diff;

    return -r * (k_SpikyGrad / (h6 * glm::max(rlen, 1e-24f))) * diff2;
}

glm::vec3 FluidConstraint::ConstraintGradient(int k, int i, int j) {
    Particle* p_i = m_allParticles->at(i);

    if (i == j) {
        glm::vec3 sum(0.0f);
        for (int n: m_neighbours[k]) {
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
    m_allParticles->at(index)->num_constraints++; // TODO
    m_neighbours.insert( {index, {}} ); // empty vector
    m_lambdas.emplace(index, 0.0f);
    m_deltas.emplace_back(0.0f);
}

float FluidConstraint::Density(int i, int k) {
    Particle *p_i = m_allParticles->at(i);
    float density = 0.0f;
    for (const int n : m_neighbours[k]) {
        Particle* p_n = m_allParticles->at(n);
        density += p_n->mass * poly6(p_i->cpos - p_n->cpos);
    }
    return density;
}

float FluidConstraint::Constraint(int i, int k) {
    const float density = Density(i, k);
    return (density / m_density) - 1.0f;
//    return density - m_density;
}
