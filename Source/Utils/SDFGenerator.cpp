#include "SDFGenerator.h"

extern Renderer *renderer;

// find distance x0 is from segment x1-x2
static float point_segment_distance(const glm::vec3 &x0, const glm::vec3 &x1, const glm::vec3 &x2)
{
    glm::vec3 dx(x2-x1);
    float m2 = glm::length2(dx);
    // find parameter value of closest point on segment
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
    // first find barycentric coordinates of closest point on infinite plane
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

glm::vec2 lineUv(glm::vec3 a, glm::vec3 b, glm::vec3 q) {
    glm::vec3 ab = b - a;
    glm::vec3 aq = q - a;
    glm::vec3 dirAb = normalize(ab);

    float v = dot(aq, dirAb) / length(ab);
    float u = 1.f - v;

    return {u, v};
}


float signedArea(glm::vec3 v1, glm::vec3 v2, glm::vec3 n) {
    float sa;

    glm::vec3 resCross = glm::cross(v1, v2);

    if (glm::length(resCross) == 0) {
        sa = 0.0f;
    } else {
        float sign = glm::dot(n, glm::normalize(resCross));
        sa = glm::length(resCross) * sign;
    }

    return sa;
}

glm::vec3 baryCoord(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 n, glm::vec3 p) {
    glm::vec3 ab = b - a;
    glm::vec3 bc = c - b;
    glm::vec3 ca = a - c;
    glm::vec3 ac = c - a;

    glm::vec3 ap = p - a;
    glm::vec3 bp = p - b;
    glm::vec3 cp = p - c;

    float Sabc = signedArea(ab, ac, n);
    float Sabp = signedArea(ab, ap, n);
    float Sbcp = signedArea(bc, bp, n);
    float Scap = signedArea(ca, cp, n);

    float u, v, w;
    u = Sbcp / Sabc;
    v = Scap / Sabc;
    w = Sabp / Sabc;

    return {u, v, w};
}

glm::vec3 point2plane(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 n, glm::vec3 p) {
    glm::vec3 ab, ac, ap;
    ap = p - a;

    // PP'
    glm::vec3 ppProj = -glm::dot(ap, n) * n;

    // AP'
    glm::vec3 apProj = ap + ppProj;

    // P'
    glm::vec3 pProj = apProj + a;

    return pProj;
}

float distPoint2Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 n, glm::vec3 p) {
    float dist = 9999.f;

    glm::vec3 Pproj = point2plane(a, b, c, n, p);
    glm::vec3 Pbary = baryCoord(a, b, c, n, Pproj);

    if ((Pbary.x >= 0 && Pbary.x <= 1.f) && (Pbary.y >= 0 && Pbary.y <= 1.f) &&
        (Pbary.z >= 0 && Pbary.z <= 1.f)) {
        glm::vec3 nm = glm::normalize(cross(b - a, c - a));

        dist = abs(dot(nm, p - a));
    }
    else {
        glm::vec2 uvAb, uvBc, uvCa;
        uvAb = lineUv(a, b, Pproj);
        uvBc = lineUv(b, c, Pproj);
        uvCa = lineUv(c, a, Pproj);

        glm::vec3 uvwAbc = baryCoord(a, b, c, n, Pproj);

        if (uvAb[1] <= 0 && uvCa[0] <= 0) {
            dist = length(p - a);
        } else if (uvAb[0] <= 0 && uvBc[1] <= 0) {
            dist = length(p - b);
        } else if (uvBc[0] <= 0 && uvCa[1] <= 0) {
            dist = length(p - c);
        }
        else if (uvAb[0] > 0 && uvAb[1] > 0 && uvwAbc[2] <= 0) {
            glm::vec3 dirAb = normalize(b - a);
            glm::vec3 APproj = Pproj - a;
            float frac = dot(APproj, dirAb);
            glm::vec3 Pinter = a + dirAb * frac;

            dist = length(p - Pinter);
        } else if (uvBc[0] > 0 && uvBc[1] > 0 && uvwAbc[0] <= 0) {
            glm::vec3 dirBc = normalize(c - b);
            glm::vec3 BPproj = Pproj - b;
            float frac = dot(BPproj, dirBc);
            glm::vec3 Pinter = b + dirBc * frac;

            dist = length(p - Pinter);
        } else if (uvCa[0] > 0 && uvCa[1] > 0 && uvwAbc[1] <= 0) {
            glm::vec3 dirCa = normalize(a - c);
            glm::vec3 CPproj = Pproj - c;
            float frac = dot(CPproj, dirCa);
            glm::vec3 Pinter = c + dirCa * frac;

            dist = length(p - Pinter);
        }
    }

    float temp = dot(p - a, n);

    float sign = (temp == 0.0f) ? 1.f : (temp / abs(temp));

    float epsilon = 0.001f;
    if (abs(temp) < epsilon) {
        sign = 1.f;
    }

    return dist * sign;
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

//    renderer->AlwaysDrawTriangle(res.p1, res.p2, res.p3, glm::vec3(0.5f));
//    renderer->AlwaysDrawTriangle(res.p4, res.p5, res.p6, glm::vec3(0.5f));
//    renderer->AlwaysDrawLine(res.p1, res.p2, glm::vec3(1.0f));
//    renderer->AlwaysDrawLine(res.p2, res.p3, glm::vec3(1.0f));
//    renderer->AlwaysDrawLine(res.p3, res.p1, glm::vec3(1.0f));
//    renderer->AlwaysDrawLine(res.p4, res.p5, glm::vec3(1.0f));
//    renderer->AlwaysDrawLine(res.p5, res.p6, glm::vec3(1.0f));
//    renderer->AlwaysDrawLine(res.p6, res.p4, glm::vec3(1.0f));

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
        float epsilon = glm::sqrt(3) * 0.25f;

        for (const tinyobj::shape_t& shape : shapes) {

            auto *rb = new RigidBody(rigidBodyID++);

            glm::vec3 minPos(INFINITY);
            glm::vec3  maxPos(-INFINITY);

            std::vector<Triangle> triangles;
            std::vector<IAABB> iaabbs;


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

            std::unordered_map<glm::vec3, float> sdf;

            for (int i = (int)minPos.x; i < (int)maxPos.x; i++) {
                for (int j = (int)minPos.y; j < (int)maxPos.y; j++) {
                    for (int k = (int)minPos.z; k < (int)maxPos.z; k++) {
                        sdf.insert(std::pair(glm::vec3(i, j, k), 99999.0f));
                    }
                }
            }

//            renderer->AlwaysDrawLineCube(minPos, maxPos - minPos, glm::vec3(0.0f));

            for (int ind = 0; ind < triangles.size(); ind++) {
                IAABB iaabb = iaabbs[ind];
                Triangle t = triangles[ind];

//                renderer->AlwaysDrawLineCube(iaabb.min, iaabb.max - iaabb.min, glm::vec3(1.0f));

//                renderer->AlwaysDrawTriangle(t.v1, t.v2, t.v3, glm::vec3(0.5f));
//                renderer->AlwaysDrawLine(t.v1 + t.n * 0.001f, t.v2 + t.n * 0.001f, glm::vec3(0.0f));
//                renderer->AlwaysDrawLine(t.v2 + t.n * 0.001f, t.v3 + t.n * 0.001f, glm::vec3(0.0f));
//                renderer->AlwaysDrawLine(t.v3 + t.n * 0.001f, t.v1 + t.n * 0.001f, glm::vec3(0.0f));
//                renderer->AlwaysDrawLine((t.v1 + t.v2 + t.v3) / 3.0f, (t.v1 + t.v2 + t.v3) / 3.0f + t.n * 0.2f, glm::vec3(0.1f, 0.8f, 0.2f));

                for (int i = iaabb.min.x; i < iaabb.max.x; i++) {
                    for (int j = iaabb.min.y; j < iaabb.max.y; j++) {
                        for (int k = iaabb.min.z; k < iaabb.max.z; k++) {
                            glm::vec3 p(i, j, k);
                            float d = point_triangle_distance(p, t.v1, t.v2, t.v3) * glm::sign(glm::dot(p - t.v1, t.n));
//                            float d = distPoint2Triangle(t.v1, t.v2, t.v3, t.n, p);

                            sdf.insert_or_assign(p, glm::abs(sdf[p]) > glm::abs(d) ? d : sdf[p]);
                        }
                    }
                }
            }

            float step = 1.0f;

            auto GetData = [&](glm::vec3 p) {
                auto GetMag = [&](glm::vec3 ps) {
                    return sdf[ps];
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

                return SDFData{glm::normalize(glm::vec3(xgrad, ygrad, zgrad)), d};
            };

            for (int i = (int)minPos.x; i < (int)maxPos.x; i++) {
                for (int j = (int)minPos.y; j < (int)maxPos.y; j++) {
                    for (int k = (int)minPos.z; k < (int)maxPos.z; k++) {
                        glm::vec3 p(i, j, k);
                        float sd = sdf[p];
                        SDFData d = GetData(p);
                        if (sd < -0.05f) {
                            auto* pt = new Particle(p, color);
                            pt->color = glm::vec3(glm::abs(sd) / 10.0f);
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

} // SDFGenerator