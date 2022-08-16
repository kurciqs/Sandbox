#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(int numParticles, ParticleSystemType type)
{
    switch (type) {
        case ParticleSystemType::Testing:
        {
            m_constraints.emplace_back();
            m_constraints.emplace_back();
            m_constraints.emplace_back();

            for (int i = 0; i < numParticles; i++) {
                auto* p = new Particle( glm::vec3(rand() % 10 - rand() % 10, 5, rand() % 10 - rand() % 10), RANDOM_COLOR );
                m_particles.push_back(p);
            }

            for (Particle* p: m_particles) {
                m_constraints[STANDARD].push_back( new BoxBoundaryConstraint(p, lowerBoundary, upperBoundary, 1.0f) );
            }

            AddCube(glm::vec3(8.0f), glm::vec3(0.0f), 4, 4, 4, RANDOM_COLOR);
            AddBall(glm::vec3(0.0f), glm::vec3(0.0f), 4.0f, RANDOM_COLOR);
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

    for (RigidBody* rb: m_rigidBodies) {
        delete rb;
    }
    m_rigidBodies.clear();
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
                // (8)
                if (p1->rigidBodyID == -1 || p2->rigidBodyID == -1) {
                    // collision between normal and rigidbody or normal and normal
                    m_constraints[CONTACT].push_back( new ContactConstraint(p1, p2, k_contact) );
                }
                else if (p1->rigidBodyID != p2->rigidBodyID) {
                    // collision between two rigid bodies, that aren't the same (their IDs aren't equal) AND they are not -1
                    SDFData d1 = m_rigidBodies[p1->rigidBodyID]->GetSDFData(i);
                    SDFData d2 = m_rigidBodies[p2->rigidBodyID]->GetSDFData(j);
//                    printf("%f  %f %f %f\n", d1.mag, d1.grad.x, d1.grad.y, d1.grad.z);
//                    printf("%f  %f %f %f\n", d2.mag, d2.grad.x, d2.grad.y, d2.grad.z);
                    m_constraints[CONTACT].push_back( new RigidContactConstraint(p1, p2, d1, d2, k_contact) );
                }
                // else do nothing p1->rigidBodyID == p2->rigidBodyID (no collision required in a rigidbody)
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
    if (!Input::isKeyDown(GLFW_KEY_G)) {
        renderer.DrawParticles(m_particles);
    }

#ifndef NDEBUG


    bool drawp = Input::isKeyDown(GLFW_KEY_R);
    if (drawp) {
        for (Particle* p : m_particles) {
            renderer.DrawLine(p->pos, glm::vec3(0.0f), glm::vec3(1.0f));
        }
    }

    bool drawc = Input::isKeyDown(GLFW_KEY_C);
    if (drawc) {
        for (int i = 0; i < m_particles.size(); i++) {
            Particle* p = m_particles[i];
            if (p->rigidBodyID != -1) {
                RigidBody* rb = m_rigidBodies[p->rigidBodyID];
                renderer.DrawLine(p->cpos + rb->GetSDFData(i).grad, p->cpos, glm::vec3(1.0f));
            }
        }
        for (const ConstraintGroup &g: m_constraints) {
            for (Constraint *c: g) {
                c->Draw(renderer);
            }
        }
    }

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
    m_constraints[STANDARD].push_back(new BoxBoundaryConstraint(p, upperBoundary, lowerBoundary, 1.0f));
}

float map(float X, float A, float B, float C, float D) {
    return (X-A)/(B-A) * (D-C) + C;
}

void ParticleSystem::AddCube(glm::vec3 pos, glm::vec3 vel, int width, int height, int depth, glm::vec3 color) {
    auto* rb = new RigidBody(m_rigidBodyCount++);

    int lastIndex = 0;
    for (int i = -width / 2; i < width / 2; i++) {
        for (int j = -height / 2; j < height / 2; j++) {
            for (int k = -depth / 2; k < depth / 2; k++) {
                glm::vec3 ppos = glm::vec3(i, j, k) + pos;

                auto *p = new Particle(ppos, color);
                p->rigidBodyID = rb->ID;
                p->vel = vel;

                // SDF Calc:
                SDFData d = {};
                rb->AddVertex(p, d, (int)m_particles.size()); // before pushing to main array
                m_particles.push_back(p);

                m_constraints[STANDARD].push_back( new BoxBoundaryConstraint(p, lowerBoundary, upperBoundary, 1.0f) );
                lastIndex++;
            }
        }
    }
    rb->CalculateOffsets();
    m_rigidBodies.push_back(rb);
    m_constraints[SHAPE].push_back( new RigidShapeConstraint(rb, k_shape) );
}

void ParticleSystem::AddBall(glm::vec3 center, glm::vec3 vel, float radius, glm::vec3 color) {
    auto* rb = new RigidBody(m_rigidBodyCount++);

    int lastIndex = 0;
    for (int i = -(int)glm::round(radius); i < (int)glm::round(radius) + 1; i++) {
        for (int j = -(int)glm::round(radius); j < (int)glm::round(radius) + 1; j++) {
            for (int k = -(int)glm::round(radius); k < (int)glm::round(radius) + 1; k++) {
                if (glm::length2(glm::vec3(i, j, k)) < radius * radius) {
                    glm::vec3 ppos = glm::vec3(i, j, k) + center;

                    auto *p = new Particle(ppos, color);
                    p->rigidBodyID = rb->ID;
                    p->vel = vel;

                    // SDF Calc:
                    glm::vec3 dir = ppos - center;
                    glm::vec3 intersection = center + glm::fastNormalize(dir) * radius;
                    glm::vec3 diff = intersection - ppos;
                    float mag = glm::fastLength(diff);
                    mag = glm::fastLength(dir) > radius ? mag : -mag;
//                    if (center == ppos) {
//                        mag = -radius + 1.0f;
//                    }

//                    p->color = glm::vec3(map(mag, 0.0f, -radius + 1.0f, 0.0f, 1.0f));
//                    printf("%f %f %f %f\n", mag, glm::normalize(diff).x, glm::normalize(diff).y, glm::normalize(diff).z);
                    SDFData d = {glm::fastNormalize(diff), mag};


                    rb->AddVertex(p, d, (int)m_particles.size()); // before pushing to main array
                    m_particles.push_back(p);

                    m_constraints[STANDARD].push_back( new BoxBoundaryConstraint(p, lowerBoundary, upperBoundary, 1.0f) );
                    lastIndex++;
                }
            }
        }
    }
    rb->CalculateOffsets();
    m_rigidBodies.push_back(rb);
    m_constraints[SHAPE].push_back( new RigidShapeConstraint(rb, k_shape) );
}