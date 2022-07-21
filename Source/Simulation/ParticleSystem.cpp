#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(int numParticles, ParticleSystemType type) {
    switch (type) {
        case ParticleSystemType::Testing:
        {

            for (int i = 0; i < numParticles; i++) {
                auto* p = new Particle( !i ? glm::vec3((float)i) : glm::vec3(rand() % 5, rand() % 5, rand() % 5), glm::vec3(0.2f, 0.4f, 0.1f) );
                m_particles.push_back(p);
            }
            m_particles[0]->fixed = true;

            ConstraintGroup g;
            for (int i = 0; i < m_particles.size(); i++) {
                if (i + 1 >= m_particles.size()) {
                    continue;
                }
                Constraint *c = new DistanceConstraint(m_particles[i], m_particles[i + 1], k_distance, 1.0f);
                g.push_back(c);
            }
            m_constraints.push_back(g);

        }
            break;
        default:
            break;
    }
}

void ParticleSystem::Clear() {
    for (auto* p : m_particles) {
        delete p;
    }
    m_particles.clear();

    for (auto& g: m_constraints) {
        for (auto *c: g) {
            delete c;
        }
        g.clear();
    }
    m_constraints.clear();
}

void ParticleSystem::Destroy() {
    Clear();
}

void ParticleSystem::Update(float dt) {
    for (Particle* p : m_particles) {
        if (Input::isMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT) && !p->fixed)
            p->ApplyForce(glm::vec3( (rand() % 100 - rand() % 100), (rand() % 100 - rand() % 100), (rand() % 100 - rand() % 100) ));

        if (!p->fixed) p->ApplyForce(m_globalForce * p->mass);

        p->vel += dt * (p->invMass * p->force);
        p->vel *= 0.98f;
        p->cpos = p->pos + (dt * p->vel);
    }

    for (int i = 0; i < SOLVER_ITERATIONS; i++) {

        for (const ConstraintGroup& g : m_constraints) {
            for (Constraint* c : g) {
                c->Project();
            }
        }

    }

    for (Particle* p : m_particles) {
        p->vel = (p->cpos - p->pos) / dt;
        p->pos = p->cpos;
        p->force = glm::vec3(0.0f);
    }
}

void ParticleSystem::Draw(Renderer& renderer) {

    bool drawp = Input::isKeyDown(GLFW_KEY_R);
    for (Particle* p : m_particles) {
        if (drawp) {
            renderer.DrawLine(p->pos, glm::vec3(0.0f), glm::vec3(1.0f));
        }
        renderer.DrawCube(p->pos - glm::vec3(0.25f), glm::vec3(0.5f), glm::vec3(0.2f, 0.4f, 0.7f));
    }
     renderer.DrawParticles(m_particles);

    bool drawc = Input::isKeyDown(GLFW_KEY_C);
    for (const ConstraintGroup& g : m_constraints) {
        for (Constraint* c : g) {
            if (drawc) {
                c->Draw(renderer);
            }
        }
    }

}


