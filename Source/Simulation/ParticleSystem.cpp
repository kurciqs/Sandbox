#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(int numParticles, ParticleSystemType type) {
    switch (type) {
        case ParticleSystemType::Testing:
        {

            for (int i = 0; i < numParticles; i++) {
                auto* p = new Particle( !i ? glm::vec3((float)i, 5.0f, (float)i) : glm::vec3(rand() % 5, rand() % 5, rand() % 5), glm::vec3(rand() % 255) / 255.0f );
                p->radius = 1.0f;
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
            // for collision / constact constraints
            m_constraints.push_back(ConstraintGroup());
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

// TODO: skip stuff if mass is 0
// TODO: counts for particles, they are how many constraints a particle is projected by
void ParticleSystem::Update(float dt) {
    // (1)
    for (Particle* p : m_particles) {
        if (Input::isMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT) && !p->fixed)
            p->ApplyForce(glm::vec3( (rand() % 100 - rand() % 100), (rand() % 100 - rand() % 100), (rand() % 100 - rand() % 100) ));

        if (!p->fixed) p->ApplyForce(m_globalForce * p->mass);

        p->vel += dt * (p->invMass * p->force);
        p->vel *= 0.98f;
        p->cpos = p->pos + (dt * p->vel);
        // TODO: (4) mass scaling
    }
    // (5)


    // (6)
    // TODO
    for (Particle* p1 : m_particles) {

        // (7) dumb, I know
        for (Particle* p2 : m_particles) {
            float dist = glm::distance(p1->cpos, p2->cpos);
            if (p1->cpos == p2->cpos)
                continue;
            if (dist < p1->radius + p2->radius) {
                // TODo: push a contact constraint here depending on the phase
                Constraint *c = new DistanceConstraint(p1, p2, k_distance, 1.0f);
                m_constraints[1].push_back(c);
            }
        }

    }

    if (Input::isKeyDown(GLFW_KEY_E))
        print_log("%d", m_constraints[1].size() + m_constraints[0].size());
    // (9)


    // (10)
    // TODO
    // (15)


    // (16)
    for (int i = 0; i < SOLVER_ITERATIONS; i++) {

        for (const ConstraintGroup& g : m_constraints) {
            for (Constraint* c : g) {
                c->Project(); // NOTE: also the counts are implemented careful
            }
        }

    }
    // (17)


    // (22)
    for (Particle* p : m_particles) {
        p->vel = (p->cpos - p->pos) / dt;
        // TODO: (25, 26)
        p->pos = p->cpos; // or apply sleeping
        p->force = glm::vec3(0.0f);
    }
    // (28)
}

void ParticleSystem::Draw(Renderer& renderer) {

    bool drawp = Input::isKeyDown(GLFW_KEY_R);
    for (Particle* p : m_particles) {
        if (drawp) {
            renderer.DrawLine(p->pos, glm::vec3(0.0f), glm::vec3(1.0f));
        }
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


