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

        return {glm::normalize(glm::vec3(xgrad, ygrad, zgrad)), d};
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

        return {glm::normalize(glm::vec3(xgrad, ygrad, zgrad)), d};
    }

    std::vector<RigidBody*> RigidBodiesFromOBJ(const std::string& path, int rigidBodyID, int firstParticleIndex, glm::vec3 offset, glm::vec3 color, glm::vec3 vel) {
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
            auto *rb = new RigidBody(rigidBodyID++);

            glm::vec3 minPos(9999.0f);
            glm::vec3 maxPos(-9999.0f);

            std::vector<tinyobj::material_t> materials_t;
            std::vector<tmd::Vec3d> vertices;  // weird vector required for library
            std::vector<std::array<int, 3>> indices;

            for (int i = 0; i < attrib.vertices.size(); i += 3) { // each time one vector
                glm::vec3 p(attrib.vertices[i + 0], attrib.vertices[i + 1], attrib.vertices[i + 2]);
                vertices.emplace_back( p.x, p.y, p.z );

                if (p.x < minPos.x) {
                    minPos.x = p.x;
                }
                if (p.y < minPos.y) {
                    minPos.y = p.y;
                }
                if (p.z < minPos.z) {
                    minPos.z = p.z;
                }
                if (p.x > maxPos.x) {
                    maxPos.x = p.x;
                }
                if (p.y > maxPos.y) {
                    maxPos.y = p.y;
                }
                if (p.z > maxPos.z) {
                    maxPos.z = p.z;
                }
            }

            size_t index_offset = 0;
            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
                auto fv = size_t(shape.mesh.num_face_vertices[f]);

                if (fv != 3) {
                    std::cerr << "Mesh: " << path << " has " << fv << " faces. Only triangles are accepted." << "\n";
                }

                indices.push_back( { shape.mesh.indices[index_offset + 0].vertex_index, shape.mesh.indices[index_offset + 1].vertex_index, shape.mesh.indices[index_offset + 2].vertex_index } );
                materials_t.push_back(materials[shape.mesh.material_ids[f]]);

                index_offset += fv;
            }

            // reshape minimum/maximum to fit entire shape
            minPos = glm::floor(minPos) - glm::vec3(1.0f);
            maxPos = glm::ceil(maxPos)  + glm::vec3(1.0f);

            tmd::TriangleMeshDistance mesh_distance(vertices, indices);
            std::unordered_map<glm::vec3, std::pair<float, Material>> sdfAndMats;

            float step = 1.0f;
            for (float i = minPos.x; i < maxPos.x; i += step) {
                for (float j = minPos.y; j < maxPos.y; j += step) {
                    for (float k = minPos.z; k < maxPos.z; k += step) {
                        glm::vec3 p(i, j, k);
                        tmd::Result res = mesh_distance.signed_distance( {p.x, p.y, p.z} );
                        sdfAndMats.insert(std::pair(p, std::pair((float)res.distance, Material{TO_VEC3(materials_t[res.triangle_id].diffuse) } )));
                    }
                }
            }

            auto GetData = [&](glm::vec3 p) {
                auto GetMag = [&](glm::vec3 ps) {
                    return sdfAndMats[ps].first;
                };

                float d = GetMag(p);
                float sign = glm::sign(d);
                float x0 = GetMag(p - glm::vec3(step, 0.0f, 0.0f));
                float x1 = GetMag(p + glm::vec3(step, 0.0f, 0.0f));
                float y0 = GetMag(p - glm::vec3(0.0f, step, 0.0f));
                float y1 = GetMag(p + glm::vec3(0.0f, step, 0.0f));
                float z0 = GetMag(p - glm::vec3(0.0f, 0.0f, step));
                float z1 = GetMag(p + glm::vec3(0.0f, 0.0f, step));

                float xgrad = sign*x0 < sign*x1 ? -(x0 - d) : (x1 - d);
                float ygrad = sign*y0 < sign*y1 ? -(y0 - d) : (y1 - d);
                float zgrad = sign*z0 < sign*z1 ? -(z0 - d) : (z1 - d);

                return SDFData{ glm::normalize(glm::vec3(xgrad, ygrad, zgrad)), d };
            };

            for (float i = minPos.x; i < maxPos.x; i += step) {
                for (float j = minPos.y; j < maxPos.y; j += step) {
                    for (float k = minPos.z; k < maxPos.z; k += step) {
                        glm::vec3 p(i, j, k);
                        SDFData d = GetData(p);
                        if (d.mag < 0.0f) {
                            auto* pt = new Particle(p + offset, color);
                            pt->radius = step / 2.0f;
                            auto dataAtPos = sdfAndMats[p];
                            if (color == glm::vec3(-1.0f)) {
                                pt->color = dataAtPos.second.diffuse;
                            }
                            pt->rigidBodyID = rb->ID;
                            pt->vel = vel;
                            rb->AddVertex(pt, d, firstParticleIndex++);
                        }
                    }
                }
            }

            rb->CalculateOffsets();
            rbs.push_back(rb);
        }

        return rbs;
    }

    SDFData SDFTorus(glm::vec3 p, glm::vec2 t, float step) {
        auto GetMag = [&](glm::vec3 ps) {
            float x = glm::length(glm::vec2(ps.x, ps.z))-t.x;
            return glm::length(glm::vec2(x, ps.y))-t.y;
        };

        float d = GetMag(p);
        float sign = glm::sign(d);
        float x0 = GetMag(p - glm::vec3(step, 0.0f, 0.0f));
        float x1 = GetMag(p + glm::vec3(step, 0.0f, 0.0f));
        float y0 = GetMag(p - glm::vec3(0.0f, step, 0.0f));
        float y1 = GetMag(p + glm::vec3(0.0f, step, 0.0f));
        float z0 = GetMag(p - glm::vec3(0.0f, 0.0f, step));
        float z1 = GetMag(p + glm::vec3(0.0f, 0.0f, step));

        float xgrad = sign*x0 < sign*x1 ? -(x0 - d) : (x1 - d);
        float ygrad = sign*y0 < sign*y1 ? -(y0 - d) : (y1 - d);
        float zgrad = sign*z0 < sign*z1 ? -(z0 - d) : (z1 - d);

        return {glm::normalize(glm::vec3(xgrad, ygrad, zgrad)), d};
    }

    SDFData SDFCylinder(glm::vec3 p, float h, float r, float step) {
        auto GetMag = [&](glm::vec3 ps) {
            glm::vec2 d = glm::abs(glm::vec2(glm::length(glm::vec2(ps.x, ps.z)),ps.y)) - glm::vec2(h,r);
            return glm::min(glm::max(d.x,d.y),0.0f) + glm::length(glm::max(d,0.0f));
        };

        float d = GetMag(p);
        float sign = glm::sign(d);
        float x0 = GetMag(p - glm::vec3(step, 0.0f, 0.0f));
        float x1 = GetMag(p + glm::vec3(step, 0.0f, 0.0f));
        float y0 = GetMag(p - glm::vec3(0.0f, step, 0.0f));
        float y1 = GetMag(p + glm::vec3(0.0f, step, 0.0f));
        float z0 = GetMag(p - glm::vec3(0.0f, 0.0f, step));
        float z1 = GetMag(p + glm::vec3(0.0f, 0.0f, step));

        float xgrad = sign*x0 < sign*x1 ? -(x0 - d) : (x1 - d);
        float ygrad = sign*y0 < sign*y1 ? -(y0 - d) : (y1 - d);
        float zgrad = sign*z0 < sign*z1 ? -(z0 - d) : (z1 - d);

        return {glm::normalize(glm::vec3(xgrad, ygrad, zgrad)), d};
    }

    SDFData SDFCone(glm::vec3 p, glm::vec2 c, float h, float step) {
        auto GetMag = [&](glm::vec3 ps) {
            glm::vec2 q = h*glm::vec2(c.x/c.y,-1.0);

            glm::vec2 w = glm::vec2( glm::length(glm::vec2(ps.x, ps.z)), ps.y );
            glm::vec2 a = w - q*glm::clamp( glm::dot(w,q)/glm::dot(q,q), 0.0f, 1.0f );
            glm::vec2 b = w - q*glm::vec2( glm::clamp( w.x/q.x, 0.0f, 1.0f ), 1.0f );
            float k = glm::sign( q.y );
            float d = glm::min(glm::dot( a, a ), glm::dot(b, b));
            float s = glm::max( k*(w.x*q.y-w.y*q.x),k*(w.y-q.y)  );
            return glm::sqrt(d)*glm::sign(s);
        };

        float d = GetMag(p);
        float sign = glm::sign(d);
        float x0 = GetMag(p - glm::vec3(step, 0.0f, 0.0f));
        float x1 = GetMag(p + glm::vec3(step, 0.0f, 0.0f));
        float y0 = GetMag(p - glm::vec3(0.0f, step, 0.0f));
        float y1 = GetMag(p + glm::vec3(0.0f, step, 0.0f));
        float z0 = GetMag(p - glm::vec3(0.0f, 0.0f, step));
        float z1 = GetMag(p + glm::vec3(0.0f, 0.0f, step));

        float xgrad = sign*x0 < sign*x1 ? -(x0 - d) : (x1 - d);
        float ygrad = sign*y0 < sign*y1 ? -(y0 - d) : (y1 - d);
        float zgrad = sign*z0 < sign*z1 ? -(z0 - d) : (z1 - d);

        return {glm::normalize(glm::vec3(xgrad, ygrad, zgrad)), d};
    }

} // SDFGenerator