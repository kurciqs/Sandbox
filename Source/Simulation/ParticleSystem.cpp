#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(int numParticles, ParticleSystemType type)
{
    switch (type) {
        case ParticleSystemType::Testing:
        {
            m_constraints.emplace_back();
            m_constraints.emplace_back();
            m_constraints.emplace_back();

            AddCube(glm::vec3(5.0f), 4, 4, 4);

            for (int i = 0; i < numParticles; i++) {
                auto* p = new Particle( glm::vec3(rand() % 10 - rand() % 10, 5, rand() % 10 - rand() % 10), RANDOM_COLOR );
                m_particles.push_back(p);

            }
            for (Particle* p: m_particles) {
                m_constraints[STANDARD].push_back(new BoxBoundaryConstraint(p, lowerBoundary, upperBoundary, 0.45f));
            }

        }
            break;
        default:
            break;
    }
}

void ParticleSystem::Clear() {
    for (auto& g: m_constraints) {
        for (auto *c: g) {
            delete c;
        }
        g.clear();
    }
    m_constraints.clear();

    // DELETE CONSTRAINTS FIRST, OTHERWISE SEGFAULTS, IDIOT
    for (auto* p : m_particles) {
        delete p;
    }
    m_particles.clear();
}

void ParticleSystem::Destroy() {
    Clear();
}

void ParticleSystem::Update(float dt) {
    // (1)
    for (Particle* p : m_particles) {
        if (Input::isMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT) && !p->fixed)
            p->ApplyForce(glm::vec3( (rand() % 100 - rand() % 100), (rand() % 100 - rand() % 100), (rand() % 100 - rand() % 100) ));
#ifdef GRAVITY
        if (!p->fixed) p->ApplyForce(m_globalForce * p->mass);
#endif
        p->vel += dt * (p->invMass * p->force);
        p->vel *= 0.98f;
        p->cpos = p->pos + (dt * p->vel);
        // TODO: (4) mass scaling (only for stiff stacks)
    }
    // (5)


    // (6)
    // TODO
    for (int i = 0; i < m_particles.size(); i++) {
        Particle* p1 = m_particles[i];

        // (7) dumb, I know
        for (int j = i + 1; j < m_particles.size(); j++) {
            Particle* p2 = m_particles[j];
            float dist = glm::fastDistance(p1->cpos, p2->cpos);
            if (dist < p1->radius + p2->radius) {
                // (8) disable collision between same phase (phase 0 collides with everyone)
                if ((!p1->phase || !p2->phase) || (p1->phase != p2->phase))
                    m_constraints[CONTACT].push_back(new ContactConstraint(p1, p2, k_contact));
            }
        }
    }
    // (9)


    // (10)
    // TODO: stabilization
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
        p->force = glm::vec3(0.0f);
        p->vel = (p->cpos - p->pos) / dt;
        // TODO: (25, 26)

        // sleeping:
        if (glm::fastDistance(p->cpos, p->pos) < PARTICLE_SLEEPING_EPSILON)
        {
            p->vel = glm::vec3(0.0f);
            continue;
        }
        p->pos = p->cpos;
    }
    // (28)

#ifdef NDEBUG
    for (Constraint* c: m_constraints[CONTACT]) {
        delete c;
    }
    m_constraints[CONTACT].clear();
#endif
}

void ParticleSystem::Draw(Renderer& renderer) {
    renderer.DrawParticles(m_particles);

#ifndef NDEBUG
    for (RigidBody* rb: m_rigidBodies) {
        rb->Draw(m_particles, renderer);
    }

    bool drawp = Input::isKeyDown(GLFW_KEY_R);
    for (Particle* p : m_particles) {
        if (drawp) {
            renderer.DrawLine(p->pos, glm::vec3(0.0f), glm::vec3(1.0f));
        }
    }

    bool drawc = Input::isKeyDown(GLFW_KEY_C);
    for (const ConstraintGroup& g : m_constraints) {
        for (Constraint* c : g) {
            if (drawc) {
                c->Draw(renderer);
            }
        }
    }

    // not cool:
    for (Constraint* c: m_constraints[CONTACT]) {
        delete c;
    }
    m_constraints[CONTACT].clear();
#endif
}

void ParticleSystem::AddParticle(glm::vec3 pos, glm::vec3 vel, glm::vec3 color, float r) {
    auto* p = new Particle(pos, color);
    p->vel = vel;
    p->radius = r;
    m_particles.push_back(p);
    m_constraints[STANDARD].push_back(new BoxBoundaryConstraint(p, upperBoundary, lowerBoundary, 0.45f));
}

void ParticleSystem::AddCube(glm::vec3 pos, int width, int height, int depth) {
    int lastIndex = 0;
    for (int i = -width / 2; i < width / 2; i++) {
        for (int j = -height / 2; j < height / 2; j++) {
            for (int k = -depth / 2; k < depth / 2; k++) {
                auto *p = new Particle(glm::vec3(i, j, k) + pos, glm::vec3(0.4f, 0.5f, 0.8f));
                p->phase = 1;
                m_particles.push_back(p);

                lastIndex++;
            }
        }
    }
    m_rigidBodies.push_back( new RigidBody(0, lastIndex - 1, m_particles) ); // TODO: offset for each new body
    m_constraints[SHAPE].push_back(new RigidShapeConstraint(m_rigidBodies[0], m_particles, 0.1f));
}


