#include "SDFGenerator.h"

extern Renderer *renderer;

static glm::vec3 GetFaceNormal(const std::vector<glm::vec3>& points_3d)
{
    // Newell's Method
    glm::vec3 n(0.0f);
    for (size_t i = 0; i < points_3d.size(); ++i)
    {
        size_t j = (i + 1) % points_3d.size();

        // http://www.opengl.org/wiki/Calculating_a_Surface_Normal
        const auto& a = points_3d[i] - points_3d[j];
        const auto& b = points_3d[i] + points_3d[j];

        n[0] += (a[1] * b[2]);
        n[1] += (a[2] * b[0]);
        n[2] += (a[0] * b[1]);
    }

    return glm::normalize(n);
}

static Prism fromTriangle(Triangle t, float epsilon) {
    Prism res{};

    res.p1 = t.v1 - t.n * epsilon;
    res.p2 = t.v2 - t.n * epsilon;
    res.p3 = t.v3 - t.n * epsilon;

    res.p4 = t.v1 + t.n * epsilon;
    res.p5 = t.v2 + t.n * epsilon;
    res.p6 = t.v3 + t.n * epsilon;

    return res;
}

static IAABB fromPrism(Prism p) {
    float minx = INFINITY;
    float maxx = -INFINITY;
    float miny = INFINITY;
    float maxy = -INFINITY;
    float minz = INFINITY;
    float maxz = -INFINITY;

    for (int i = 0; i < 6; i++) {
        glm::vec3 ps = p[i];
        if (ps.x < minx) {
            minx = ps.x;
        }
        if (ps.y < miny) {
            miny = ps.y;
        }
        if (ps.z < minz) {
            minz = ps.z;
        }
        if (ps.x > maxx) {
            maxx = ps.x;
        }
        if (ps.y > maxy) {
            maxy = ps.y;
        }
        if (ps.z > maxz) {
            maxz = ps.z;
        }
    }

    return IAABB{glm::ceil(glm::vec3(minx, miny, minz)) - 1.0f, glm::floor(glm::vec3(maxx, maxy, maxz)) + 1.0f};
}

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

    std::vector<RigidBody*> RigidBodiesFromOBJ(const std::string& path, int rigidBodyID, int firstParticleIndex, glm::vec3 offset) {
        tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path = "Assets/Models/"; // Path to material files

        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(path, reader_config)) {
            if (!reader.Error().empty()) {
                std::cerr << "TinyObjReader: " << reader.Error();
            }
        }

        if (!reader.Warning().empty()) {
            std::cout << "TinyObjReader: " << reader.Warning();
        }

        auto& attrib = reader.GetAttrib();
        auto& shapes = reader.GetShapes();
        auto& materials = reader.GetMaterials();

        std::vector<RigidBody*> rbs;

        for (const tinyobj::shape_t& shape : shapes) {

//            auto *rb = new RigidBody(rigidBodyID++);

            std::vector<Triangle> triangles;

            size_t index_offset = 0;
            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
                auto fv = size_t(shape.mesh.num_face_vertices[f]);

                if (fv != 3) {
                    std::cerr << "Mesh: " << path << " has " << fv << " faces. Only triangles are accepted." << "\n";
                }

                Triangle triangle{};
                tinyobj::index_t idx = shape.mesh.indices[index_offset];
                triangle.v1 = {
                        attrib.vertices[3 * size_t(idx.vertex_index) + 0],
                        attrib.vertices[3 * size_t(idx.vertex_index) + 1],
                        attrib.vertices[3 * size_t(idx.vertex_index) + 2]};

                idx = shape.mesh.indices[index_offset + 1];
                triangle.v2 = {
                        attrib.vertices[3 * size_t(idx.vertex_index) + 0],
                        attrib.vertices[3 * size_t(idx.vertex_index) + 1],
                        attrib.vertices[3 * size_t(idx.vertex_index) + 2]};

                idx = shape.mesh.indices[index_offset + 2];
                triangle.v3 = {
                        attrib.vertices[3 * size_t(idx.vertex_index) + 0],
                        attrib.vertices[3 * size_t(idx.vertex_index) + 1],
                        attrib.vertices[3 * size_t(idx.vertex_index) + 2]};

                triangle.n = GetFaceNormal( {triangle.v1, triangle.v2, triangle.v3} );

                triangles.push_back(triangle);

                index_offset += fv;
            }

            float epsilon = glm::sqrt(3);
            for (Triangle t: triangles) {
                Prism prism = fromTriangle(t, epsilon);
                IAABB iaabb = fromPrism(prism);

                std::map<glm::ivec3, float> signedDistanceField;

                renderer->DrawTriangle(t.v1, t.v2, t.v3, glm::vec3(0.5f));
                renderer->DrawLine(t.v1 + t.n * 0.001f, t.v2 + t.n * 0.001f, glm::vec3(0.0f));
                renderer->DrawLine(t.v2 + t.n * 0.001f, t.v3 + t.n * 0.001f, glm::vec3(0.0f));
                renderer->DrawLine(t.v3 + t.n * 0.001f, t.v1 + t.n * 0.001f, glm::vec3(0.0f));
                renderer->DrawLine((t.v1 + t.v2 + t.v3) / 3.0f, (t.v1 + t.v2 + t.v3) / 3.0f + t.n * 0.2f, glm::vec3(0.1f, 0.8f, 0.2f));

                for (int i = iaabb.min.x; i < iaabb.max.x; i++) {
                    for (int j = iaabb.min.y; j < iaabb.max.y; j++) {
                        for (int k = iaabb.min.z; k < iaabb.max.z; k++) {
                            glm::vec3 p(i, j, k);
                            float d = glm::dot(p - t.v1, t.n);
                            float sgn = glm::sign(d);
                            float distToV1 = glm::distance(p, t.v1);
                            float distToV2 = glm::distance(p, t.v2);
                            float distToV3 = glm::distance(p, t.v3);
                            float minDist = glm::min(distToV1, glm::min(distToV2, distToV3));
                            // TODO

                        }
                    }
                }
            }

//            rb->CalculateOffsets();

//            rbs.push_back(rb);
        }

        return rbs;
    }

} // SDFGenerator