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
//                m_constraints[CONTACT].push_back( new ContactConstraint(p1, p2, k_contact) );
                // (8) TODO improved collision between rbs:
                if (p1->rigidBodyID == -1 || p2->rigidBodyID == -1) {
                    m_constraints[CONTACT].push_back( new ContactConstraint(p1, p2, k_contact) );
                }
                else if (p1->rigidBodyID != p2->rigidBodyID) {
                    SDFData d1 = m_rigidBodies[p1->rigidBodyID]->GetSDFData(i);
                    SDFData d2 = m_rigidBodies[p2->rigidBodyID]->GetSDFData(j);
                    m_constraints[CONTACT].push_back( new RigidContactConstraint(p1, p2, d1, d2, k_contact) );
                }

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
//        for (Particle* p : m_particles) {
//            renderer.DrawLine(p->pos, glm::vec3(0.0f), glm::vec3(1.0f));
//        }
        for (int i = 0; i < m_particles.size(); i++) {
            Particle* p = m_particles[i];
            if (p->rigidBodyID != -1) {
                RigidBody* rb = m_rigidBodies[p->rigidBodyID];
                renderer.DrawLine(p->cpos + rb->GetSDFData(i).grad, p->cpos, glm::vec3(1.0f));
//                renderer.DrawLine(p->cpos + rb->GetSDFData(i).grad * glm::abs(rb->GetSDFData(i).mag), p->cpos, glm::vec3(0.0f, 0.2f, 0.4f));
            }
        }
    }

    bool drawc = Input::isKeyDown(GLFW_KEY_C);
    if (drawc) {

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

float SDFCube(glm::vec3 p, glm::vec3 b) {
    glm::vec3 d = glm::abs(p) - b;
    return glm::min(glm::max(d.x,glm::max(d.y,d.z)),0.0f) + glm::length(glm::max(d,0.0f));
}

glm::vec3 SDFGradientCube(glm::vec3 p, glm::vec3 b, bool corner) {
    float d = SDFCube(p, b);
    float sign = d >= 0 ? 1.0f : -1.0f;
    float x0 = SDFCube(p - glm::vec3(1.0f, 0.0f, 0.0f), b);
    float x1 = SDFCube(p + glm::vec3(1.0f, 0.0f, 0.0f), b);
    float y0 = SDFCube(p - glm::vec3(0.0f, 1.0f, 0.0f), b);
    float y1 = SDFCube(p + glm::vec3(0.0f, 1.0f, 0.0f), b);
    float z0 = SDFCube(p - glm::vec3(0.0f, 0.0f, 1.0f), b);
    float z1 = SDFCube(p + glm::vec3(0.0f, 0.0f, 1.0f), b);

    float xgrad;
    float ygrad;
    float zgrad;

    if (corner) {
        xgrad = sign * x0 < sign * x1 ? (x1 - d) : -(x0 - d);
        ygrad = sign * y0 < sign * y1 ? (y1 - d) : -(y0 - d);
        zgrad = sign * z0 < sign * z1 ? (z1 - d) : -(z0 - d);
    }
    else {
        xgrad = sign * x0 > sign * x1 ? (x1 - d) : -(x0 - d);
        ygrad = sign * y0 > sign * y1 ? (y1 - d) : -(y0 - d);
        zgrad = sign * z0 > sign * z1 ? (z1 - d) : -(z0 - d);
    }

    return glm::fastNormalize(glm::vec3(xgrad, ygrad, zgrad));
}

void ParticleSystem::AddCube(glm::vec3 pos, glm::vec3 vel, int width, int height, int depth, glm::vec3 color) {
    auto* rb = new RigidBody(m_rigidBodyCount++);

    glm::vec3 boxSize((float)width / 2.0f,  (float)height / 2.0f, (float)depth / 2.0f);
    float step = 1.0f;
    for (float i = -boxSize.x; i <= boxSize.y; i += step) {
        for (float j = -boxSize.y; j <= boxSize.y; j += step) {
            for (float k = -boxSize.z; k <= boxSize.z; k += step) {
                glm::vec3 ppos = glm::vec3(i, j, k) + pos;

                auto *p = new Particle(ppos, color);
                p->rigidBodyID = rb->ID;
                p->vel = vel;

                // SDF Calc:
                glm::vec3 local = ppos - pos;
                bool corner = (glm::abs(local.x) == boxSize.x && glm::abs(local.y) == boxSize.y) || (glm::abs(local.y) == boxSize.y && glm::abs(local.z) == boxSize.z) || (glm::abs(local.x) == boxSize.x && glm::abs(local.z) == boxSize.z);
                SDFData d = {SDFGradientCube(local, boxSize, corner), SDFCube(local, boxSize)};

                rb->AddVertex(p, d, (int)m_particles.size()); // before pushing to main array
                m_particles.push_back(p);

                m_constraints[STANDARD].push_back( new BoxBoundaryConstraint(p, lowerBoundary, upperBoundary, 1.0f) );
            }
        }
    }
    rb->CalculateOffsets();
    m_rigidBodies.push_back(rb);
    m_constraints[SHAPE].push_back( new RigidShapeConstraint(rb, k_shape) );
}

float SDFBall(glm::vec3 p, float s) {
    return glm::length(p) - s;
}

glm::vec3 SDFGradientBall(glm::vec3 p, float s) {
    float d = SDFBall(p, s);
    float sign = glm::sign(d);
    float x0 = SDFBall(p - glm::vec3(1.0f, 0.0f, 0.0f), s);
    float x1 = SDFBall(p + glm::vec3(1.0f, 0.0f, 0.0f), s);
    float y0 = SDFBall(p - glm::vec3(0.0f, 1.0f, 0.0f), s);
    float y1 = SDFBall(p + glm::vec3(0.0f, 1.0f, 0.0f), s);
    float z0 = SDFBall(p - glm::vec3(0.0f, 0.0f, 1.0f), s);
    float z1 = SDFBall(p + glm::vec3(0.0f, 0.0f, 1.0f), s);

    float xgrad = sign*x0 < sign*x1 ? -(x0 - d) : (x1 - d);
    float ygrad = sign*y0 < sign*y1 ? -(y0 - d) : (y1 - d);
    float zgrad = sign*z0 < sign*z1 ? -(z0 - d) : (z1 - d);

    return glm::fastNormalize(glm::vec3(xgrad, ygrad, zgrad));
}

void ParticleSystem::AddBall(glm::vec3 center, glm::vec3 vel, float radius, glm::vec3 color) {
    auto* rb = new RigidBody(m_rigidBodyCount++);

    for (int i = -(int)glm::round(radius); i < (int)glm::round(radius) + 1; i++) {
        for (int j = -(int)glm::round(radius); j < (int)glm::round(radius) + 1; j++) {
            for (int k = -(int)glm::round(radius); k < (int)glm::round(radius) + 1; k++) {
                if (glm::length2(glm::vec3(i, j, k)) < radius * radius) {
                    glm::vec3 ppos = glm::vec3(i, j, k) + center;

                    auto *p = new Particle(ppos, color);
                    p->rigidBodyID = rb->ID;
                    p->vel = vel;

                    glm::vec3 local = ppos - center;
                    SDFData d = {SDFGradientBall(local, radius), SDFBall(local, radius)};

                    rb->AddVertex(p, d, (int)m_particles.size()); // before pushing to main array
                    m_particles.push_back(p);

                    m_constraints[STANDARD].push_back( new BoxBoundaryConstraint(p, lowerBoundary, upperBoundary, 1.0f) );
                }
            }
        }
    }
    rb->CalculateOffsets();
    m_rigidBodies.push_back(rb);
    m_constraints[SHAPE].push_back( new RigidShapeConstraint(rb, k_shape) );
}