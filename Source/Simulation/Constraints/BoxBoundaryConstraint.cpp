#include "BoxBoundaryConstraint.h"

BoxBoundaryConstraint::BoxBoundaryConstraint(Particle *p1, glm::vec3 min, glm::vec3 max, float k) {
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);
    m_min = min;
    m_max = max;
    m_particles.push_back(p1);
    IncrementCounts();
}

BoxBoundaryConstraint::~BoxBoundaryConstraint() {
    DecrementCounts();
}

void BoxBoundaryConstraint::Project() {
    Particle* p1 = m_particles[0];

    glm::vec3 pos = p1->cpos;
    glm::vec3 center = (m_min + m_max) / 2.0f;
    glm::vec3 size = glm::abs(m_min - center);
    pos -= center;

    auto GetMag = [&](glm::vec3 ps, glm::vec3 s) {
        glm::vec3 d = glm::abs(ps) - s;
        return glm::min(glm::max(d.x,glm::max(d.y,d.z)),0.0f) + glm::fastLength(glm::max(d,0.0f)) + p1->radius;
    };

    float step = 0.1f;
    float d = GetMag(pos, size);

    if (d <= 0.0f) {
        return;
    }

    float sign = d >= 0 ? 1.0f : -1.0f;
    float x0 = GetMag(pos - glm::vec3(step, 0.0f, 0.0f), size);
    float x1 = GetMag(pos + glm::vec3(step, 0.0f, 0.0f), size);
    float y0 = GetMag(pos - glm::vec3(0.0f, step, 0.0f), size);
    float y1 = GetMag(pos + glm::vec3(0.0f, step, 0.0f), size);
    float z0 = GetMag(pos - glm::vec3(0.0f, 0.0f, step), size);
    float z1 = GetMag(pos + glm::vec3(0.0f, 0.0f, step), size);

    float xgrad = sign * x0 > sign * x1 ? (x1 - d) : -(x0 - d);
    float ygrad = sign * y0 > sign * y1 ? (y1 - d) : -(y0 - d);
    float zgrad = sign * z0 > sign * z1 ? (z1 - d) : -(z0 - d);

    glm::vec3 dir = -glm::normalize( glm::vec3(xgrad, ygrad, zgrad) );

    if (!p1->fixed) p1->cpos += SOR_COEF * dir * d * m_stiffness;
}

void BoxBoundaryConstraint::Draw(Renderer &renderer) {
    renderer.DrawLineCube(m_min, m_max - m_min, glm::vec3(0.0f, 0.4f, 0.8f));
}
