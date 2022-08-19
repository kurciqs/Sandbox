#include "SDFGenerator.h"

namespace Generator {
    SDFData SDFCube(glm::vec3 p, glm::vec3 b, float step) {
        auto GetMag = [&](glm::vec3 ps, glm::vec3 s) {
            glm::vec3 d = glm::abs(ps) - s;
            return glm::min(glm::max(d.x,glm::max(d.y,d.z)),0.0f) + glm::length(glm::max(d,0.0f));
        };

        float d = GetMag(p, b);
        float sign = d >= 0 ? 1.0f : -1.0f;
        float x0 = GetMag(p - glm::vec3(step, 0.0f, 0.0f), b);
        float x1 = GetMag(p + glm::vec3(step, 0.0f, 0.0f), b);
        float y0 = GetMag(p - glm::vec3(0.0f, step, 0.0f), b);
        float y1 = GetMag(p + glm::vec3(0.0f, step, 0.0f), b);
        float z0 = GetMag(p - glm::vec3(0.0f, 0.0f, step), b);
        float z1 = GetMag(p + glm::vec3(0.0f, 0.0f, step), b);

        float xgrad = sign * x0 > sign * x1 ? (x1 - d) : -(x0 - d);
        float ygrad = sign * y0 > sign * y1 ? (y1 - d) : -(y0 - d);
        float zgrad = sign * z0 > sign * z1 ? (z1 - d) : -(z0 - d);

        return {(glm::vec3(xgrad, ygrad, zgrad)), d};
    }

    SDFData SDFBall(glm::vec3 p, float s, float step) {
        auto GetMag = [&](glm::vec3 ps, float s) {
            return glm::length(ps) - s;
        };

        float d = GetMag(p, s);
        float sign = glm::sign(d);
        float x0 = GetMag(p - glm::vec3(step, 0.0f, 0.0f), s);
        float x1 = GetMag(p + glm::vec3(step, 0.0f, 0.0f), s);
        float y0 = GetMag(p - glm::vec3(0.0f, step, 0.0f), s);
        float y1 = GetMag(p + glm::vec3(0.0f, step, 0.0f), s);
        float z0 = GetMag(p - glm::vec3(0.0f, 0.0f, step), s);
        float z1 = GetMag(p + glm::vec3(0.0f, 0.0f, step), s);

        float xgrad = sign*x0 < sign*x1 ? -(x0 - d) : (x1 - d);
        float ygrad = sign*y0 < sign*y1 ? -(y0 - d) : (y1 - d);
        float zgrad = sign*z0 < sign*z1 ? -(z0 - d) : (z1 - d);

        return {glm::vec3(xgrad, ygrad, zgrad), d};
    }

    RigidBody *RigidBodyFromOBJ(const std::string& path, int rigidBodyID, int firstParticleIndex, glm::vec3 offset) {
        auto* rb = new RigidBody(rigidBodyID);

        tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path = "Assets/Models/"; // Path to material files

        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(path, reader_config)) {
            if (!reader.Error().empty()) {
                std::cerr << "TinyObjReader: " << reader.Error();
            }
            exit(1);
        }

        if (!reader.Warning().empty()) {
            std::cout << "TinyObjReader: " << reader.Warning();
        }

        auto& attrib = reader.GetAttrib();
        auto& shapes = reader.GetShapes();
        auto& materials = reader.GetMaterials();
        std::vector<Vertex> vertices;
        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    tinyobj::real_t vx = attrib.vertices[3*size_t(idx.vertex_index)+0];
                    tinyobj::real_t vy = attrib.vertices[3*size_t(idx.vertex_index)+1];
                    tinyobj::real_t vz = attrib.vertices[3*size_t(idx.vertex_index)+2];

                    // Check if `normal_index` is zero or positive. negative = no normal data
                    if (idx.normal_index >= 0) {
                        tinyobj::real_t nx = attrib.normals[3*size_t(idx.normal_index)+0];
                        tinyobj::real_t ny = attrib.normals[3*size_t(idx.normal_index)+1];
                        tinyobj::real_t nz = attrib.normals[3*size_t(idx.normal_index)+2];
                    }
                }
                index_offset += fv;
            }
        }

        for (int i = 0; i < attrib.vertices.size(); i += 3) {
            tinyobj::real_t vx = attrib.vertices[size_t(i)+0];
            tinyobj::real_t vy = attrib.vertices[size_t(i)+1];
            tinyobj::real_t vz = attrib.vertices[size_t(i)+2];

            glm::vec3 pos(vx, vy, vz);
            auto* p = new Particle(pos + offset, glm::vec3(0.5f));

            tinyobj::real_t nx = attrib.normals[size_t(i)+0];
            tinyobj::real_t ny = attrib.normals[size_t(i)+1];
            tinyobj::real_t nz = attrib.normals[size_t(i)+2];

            SDFData d{{nx, ny, nz}, 1.0f};
            printf("%f %f %f \n", d.grad.x, d.grad.y, d.grad.z);

            rb->AddVertex(p, d, firstParticleIndex++); // before pushing to main array
        }

        return rb;
    }

} // SDFGenerator