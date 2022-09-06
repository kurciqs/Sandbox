#include "SDFGenerator.h"

extern Renderer *renderer;

static bool IS_EDGE(glm::vec3 p, std::unordered_map<glm::vec3, float>& sdf) {
    bool is_outer = IS_OUTER(sdf[p]);

    // check all surrounding pixels
    for (float x = -1.0f; x <= 1.0f; x += 1.0f) {
        for (float y = -1.0f; y <= 1.0f; y += 1.0f) {
            for (float z = -1.0f; z <= 1.0f; z += 1.0f) {
                glm::vec3 newPos = p + glm::vec3(x, y, z);
                if (is_outer != IS_OUTER(sdf[newPos])) {
                    return true;
                }
            }
        }
    }
    return false;
}

// find distance x0 is from segment x1-x2
static float point_segment_distance(const glm::vec3 &x0, const glm::vec3 &x1, const glm::vec3 &x2)
{
    glm::vec3 dx(x2-x1);
    float m2 = glm::length2(dx);
    // find parameter value of the closest point on segment
    float s12=(float)(glm::dot(x2-x0, dx)/m2);
    if(s12<0.0f){
        s12=0.0f;
    }else if(s12>1.0f){
        s12=1.0f;
    }
    // and find the distance
    return glm::distance(x0, s12*x1+(1-s12)*x2);
}

// find distance x0 is from triangle x1-x2-x3
static float point_triangle_distance(const glm::vec3 &x0, const glm::vec3 &x1, const glm::vec3 &x2, const glm::vec3 &x3)
{
    // first find barycentric coordinates of the closest point on infinite plane
    glm::vec3 x13(x1-x3), x23(x2-x3), x03(x0-x3);
    float m13=glm::length2(x13), m23=glm::length2(x23), d=dot(x13,x23);
    float invdet=1.f/glm::max(m13*m23-d*d,1e-30f);
    float a=glm::dot(x13,x03), b=glm::dot(x23,x03);
    // the barycentric coordinates themselves
    float w23=invdet*(m23*a-d*b);
    float w31=invdet*(m13*b-d*a);
    float w12=1.0f-w23-w31;
    if(w23>=0.0f && w31>=0.0f && w12>=0.0f){ // if we're inside the triangle
        return glm::distance(x0, w23*x1+w31*x2+w12*x3);
    }else{ // we have to clamp to one of the edges
        if(w23>0.0f) // this rules out edge 2-3 for us
            return glm::min(point_segment_distance(x0,x1,x2), point_segment_distance(x0,x1,x3));
        else if(w31>0) // this rules out edge 1-3
            return glm::min(point_segment_distance(x0,x1,x2), point_segment_distance(x0,x2,x3));
        else // w12 must be >0, ruling out edge 1-2
            return glm::min(point_segment_distance(x0,x1,x3), point_segment_distance(x0,x2,x3));
    }
}

static glm::vec3 getFaceNormal(const std::vector<glm::vec3>& points_3d)
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

    res.p1 = t.v1 + t.n * epsilon;
    res.p2 = t.v2 + t.n * epsilon;
    res.p3 = t.v3 + t.n * epsilon;

    res.p4 = t.v1 - t.n * epsilon;
    res.p5 = t.v2 - t.n * epsilon;
    res.p6 = t.v3 - t.n * epsilon;
/*
    renderer->AlwaysDrawTriangle(res.p1, res.p2, res.p3, glm::vec3(0.5f));
    renderer->AlwaysDrawTriangle(res.p4, res.p5, res.p6, glm::vec3(0.5f));
    renderer->AlwaysDrawLine(res.p1, res.p2, glm::vec3(1.0f));
    renderer->AlwaysDrawLine(res.p2, res.p3, glm::vec3(1.0f));
    renderer->AlwaysDrawLine(res.p3, res.p1, glm::vec3(1.0f));
    renderer->AlwaysDrawLine(res.p4, res.p5, glm::vec3(1.0f));
    renderer->AlwaysDrawLine(res.p5, res.p6, glm::vec3(1.0f));
    renderer->AlwaysDrawLine(res.p6, res.p4, glm::vec3(1.0f));
*/
    return res;
}

static IAABB fromPrism(Prism p) {
    float minx = 9999.0f;
    float maxx = -9999.0f;
    float miny = 9999.0f;
    float maxy = -9999.0f;
    float minz = 9999.0f;
    float maxz = -9999.0f;

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

    IAABB iaabb{glm::floor(glm::vec3(minx, miny, minz)), glm::ceil(glm::vec3(maxx, maxy, maxz))};
//    renderer->AlwaysDrawLineCube(iaabb.min, iaabb.max - iaabb.min, RANDOM_COLOR);
    return iaabb;
}

static float distanceToEdge(glm::vec3 P0, glm::vec3 P1, glm::vec3 P)
{
    glm::vec3 v = P1 - P0;
    glm::vec3 w = P - P0;
    float c1 = glm::dot(w, v);
    float c2 = glm::dot(v, v);
    float b = c1 / c2;
    return glm::length(P - (P0 + b * v));
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
        float epsilon = glm::sqrt(3) * 0.75f;

        for (const tinyobj::shape_t& shape : shapes) {

            auto *rb = new RigidBody(rigidBodyID++);

            glm::vec3 minPos(-3.0f);
            glm::vec3 maxPos(3.0f);
/*
            std::vector<Triangle> triangles;
            std::vector<IAABB> iaabbs;
            std::vector<tinyobj::material_t> materials_t;
*/

            // Declare mesh vertices and triangles
            std::vector<tmd::Vec3d> vertices;
            std::vector<std::array<int, 3>> indices;
            // indices[i] are inds to a triangle which pulls positions from vertices

            for (int i = 0; i < attrib.vertices.size(); i += 3) {
                vertices.emplace_back( attrib.vertices[i + 0], attrib.vertices[i + 1], attrib.vertices[i + 2] );
            }

            for (int i = 0; i < shape.mesh.indices.size(); i += 3) {
                indices.push_back( { shape.mesh.indices[i + 0].vertex_index, shape.mesh.indices[i + 1].vertex_index, shape.mesh.indices[i + 2].vertex_index } );
            }
/*
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

                triangle.n = getFaceNormal({triangle.v1, triangle.v2, triangle.v3});

                triangles.push_back(triangle);
                materials_t.push_back(materials[shape.mesh.material_ids[f]]);

                Prism prism = fromTriangle(triangle, epsilon);
                IAABB iaabb = fromPrism(prism);

                if ((float)iaabb.min.x < minPos.x) {
                    minPos.x = (float)iaabb.min.x;
                }
                if ((float)iaabb.min.y < minPos.y) {
                    minPos.y = (float)iaabb.min.y;
                }
                if ((float)iaabb.min.z < minPos.z) {
                    minPos.z = (float)iaabb.min.z;
                }
                if ((float)iaabb.max.x > maxPos.x) {
                    maxPos.x = (float)iaabb.max.x;
                }
                if ((float)iaabb.max.y > maxPos.y) {
                    maxPos.y = (float)iaabb.max.y;
                }
                if ((float)iaabb.max.z > maxPos.z) {
                    maxPos.z = (float)iaabb.max.z;
                }

                iaabbs.push_back(iaabb);
                index_offset += fv;
            }
*/
/*
            std::unordered_map<glm::vec3, float> sdf;
            std::unordered_map<glm::vec3, glm::vec3> colors;
            for (int i = (int)minPos.x; i < (int)maxPos.x; i++) {
                for (int j = (int)minPos.y; j < (int)maxPos.y; j++) {
                    for (int k = (int)minPos.z; k < (int)maxPos.z; k++) {
                        sdf.insert(std::pair(glm::vec3(i, j, k), 9999.0f));
                    }
                }
            }
*/
/*
            for (int ind = 0; ind < triangles.size(); ind++) {
                IAABB iaabb = iaabbs[ind];
                Triangle t = triangles[ind];
                tinyobj::material_t material = materials_t[ind];

                renderer->AlwaysDrawLineCube(glm::vec3(iaabb.min) + glm::vec3(0.05f), glm::vec3(iaabb.max - iaabb.min) - glm::vec3(0.05f), RANDOM_COLOR);


                renderer->AlwaysDrawTriangle(t.v1, t.v2, t.v3, glm::vec3(0.5f));
                renderer->AlwaysDrawLine(t.v1 + t.n * 0.001f, t.v2 + t.n * 0.001f, glm::vec3(0.0f));
                renderer->AlwaysDrawLine(t.v2 + t.n * 0.001f, t.v3 + t.n * 0.001f, glm::vec3(0.0f));
                renderer->AlwaysDrawLine(t.v3 + t.n * 0.001f, t.v1 + t.n * 0.001f, glm::vec3(0.0f));
                renderer->AlwaysDrawLine((t.v1 + t.v2 + t.v3) / 3.0f, (t.v1 + t.v2 + t.v3) / 3.0f + t.n * 0.2f, glm::vec3(0.1f, 0.8f, 0.2f));


                // SDF part
                for (int i = iaabb.min.x; i < iaabb.max.x; i++) {
                    for (int j = iaabb.min.y; j < iaabb.max.y; j++) {
                        for (int k = iaabb.min.z; k < iaabb.max.z; k++) {
                            glm::vec3 p(i, j, k);

                            float d = point_triangle_distance(p, t.v1, t.v2, t.v3);

                            float dotp = glm::dot(p - t.v1, t.n);
                            float sign = (dotp == 0.0f) ? 1.0f : dotp / glm::abs(dotp);
                            if (glm::abs(dotp) < 0.01f) {
                                sign = 1.f;
                            }

//                            renderer->AlwaysDrawLineCube(p - glm::vec3(0.5f), glm::vec3(1.0f), glm::vec3(1.0f));

                            sdf.insert_or_assign(p, glm::abs(sdf[p]) > glm::abs(d) ? sign * d : sdf[p]);

                            colors.insert_or_assign(p, glm::vec3(material.diffuse[0], material.diffuse[1], material.diffuse[2]));
                        }
                    }
                }
            }
*/
/*
            for (int i = (int)minPos.x; i < (int)maxPos.x; i++) {
                for (int j = (int)minPos.y; j < (int)maxPos.y; j++) {
                    for (int k = (int)minPos.z; k < (int)maxPos.z; k++) {
                        float dist = 9999.0f;
                        int nearestTriangle;
                        glm::vec3 p(i, j, k);
                        for (int ind = 0; ind < triangles.size(); ind++) {
                            Triangle t = triangles[ind];

                            float temp = point_triangle_distance(p, t.v1, t.v2, t.v3);
                            float dotp = glm::dot(p - t.v1, t.n);
                            float sign = (dotp == 0.0f) ? 1.0f : dotp / glm::abs(dotp);

                            if (glm::abs(dotp) < 0.25f) {
                                sign = 1.f;
                            }

                            if (glm::abs(temp) < glm::abs(dist)) {
                                dist = temp * sign;
                                nearestTriangle = ind;
                            }
                        }
                        colors.insert_or_assign(p, glm::vec3(materials_t[nearestTriangle].diffuse[0], materials_t[nearestTriangle].diffuse[1], materials_t[nearestTriangle].diffuse[2]));
                        sdf.insert_or_assign(p, dist);
                    }
                }
            }
*/


            tmd::TriangleMeshDistance mesh_distance(vertices, indices);

            float step = 1.0f;
            auto GetData = [&](glm::vec3 p) {
                auto GetMag = [&](glm::vec3 ps) {
                    return (float)mesh_distance.signed_distance({ ps.x, ps.y, ps.z }).distance;
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

            for (int i = (int)minPos.x; i < (int)maxPos.x; i++) {
                for (int j = (int)minPos.y; j < (int)maxPos.y; j++) {
                    for (int k = (int)minPos.z; k < (int)maxPos.z; k++) {
                        glm::vec3 p(i, j, k);
                        SDFData d = GetData(p);
//                        printf("%f\n", d.mag);
//                        renderer->AlwaysDrawLineCube(p - glm::vec3(0.4f), glm::vec3(0.8f), glm::sign(d.mag) == 1.0f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(glm::abs(d.mag) / 4.0f));
                        if (d.mag <= 0.0f) {
                            auto* pt = new Particle(p, color);
                            pt->color = color;
//                            pt->color = glm::vec3(glm::abs(d.mag) / 10.0f);
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

} // SDFGenerator