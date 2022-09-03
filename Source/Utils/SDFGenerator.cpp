#include "SDFGenerator.h"

extern Renderer *renderer;

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

static int orientation(float x1, float y1, float x2, float y2, float &twice_signed_area)
{
    twice_signed_area=y1*x2-x1*y2;
    if(twice_signed_area>0) return 1;
    else if(twice_signed_area<0) return -1;
    else if(y2>y1) return 1;
    else if(y2<y1) return -1;
    else if(x1>x2) return 1;
    else if(x1<x2) return -1;
    else return 0; // only true when x1==x2 and y1==y2
}

// robust test of (x0,y0) in the triangle (x1,y1)-(x2,y2)-(x3,y3)
// if true is returned, the barycentric coordinates are set in a,b,c.
static bool point_in_triangle_2d(float x0, float y0,
                                 float x1, float y1, float x2, float y2, float x3, float y3,
                                 float& a, float& b, float& c)
{
    x1-=x0; x2-=x0; x3-=x0;
    y1-=y0; y2-=y0; y3-=y0;
    int signa=orientation(x2, y2, x3, y3, a);
    if(signa==0) return false;
    int signb=orientation(x3, y3, x1, y1, b);
    if(signb!=signa) return false;
    int signc=orientation(x1, y1, x2, y2, c);
    if(signc!=signa) return false;
    float sum=a+b+c;
    assert(sum!=0); // if the SOS signs match and are nonkero, there's no way all of a, b, and c are zero.
    a/=sum;
    b/=sum;
    c/=sum;
    return true;
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
            glm::vec3 maxPos(-INFINITY);

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
                        sdf.insert(std::pair(glm::vec3(i, j, k), 9999.0f));
                    }
                }
            }

//            renderer->AlwaysDrawLineCube(minPos, maxPos - minPos, glm::vec3(0.0f));

            int intersections[1000][1000][1000];
            glm::ivec3 size(glm::abs(maxPos.x - minPos.x), glm::abs(maxPos.y - minPos.y), glm::abs(maxPos.z - minPos.z));

            for (int ind = 0; ind < triangles.size(); ind++) {
                IAABB iaabb = iaabbs[ind];
                Triangle t = triangles[ind];

//                renderer->AlwaysDrawLineCube(iaabb.min, iaabb.max - iaabb.min, glm::vec3(1.0f));

//                renderer->AlwaysDrawTriangle(t.v1, t.v2, t.v3, glm::vec3(0.5f));
//                renderer->AlwaysDrawLine(t.v1 + t.n * 0.001f, t.v2 + t.n * 0.001f, glm::vec3(0.0f));
//                renderer->AlwaysDrawLine(t.v2 + t.n * 0.001f, t.v3 + t.n * 0.001f, glm::vec3(0.0f));
//                renderer->AlwaysDrawLine(t.v3 + t.n * 0.001f, t.v1 + t.n * 0.001f, glm::vec3(0.0f));
//                renderer->AlwaysDrawLine((t.v1 + t.v2 + t.v3) / 3.0f, (t.v1 + t.v2 + t.v3) / 3.0f + t.n * 0.2f, glm::vec3(0.1f, 0.8f, 0.2f));
                // SDF part
                for (int i = iaabb.min.x; i < iaabb.max.x; i++) {
                    for (int j = iaabb.min.y; j < iaabb.max.y; j++) {
                        for (int k = iaabb.min.z; k < iaabb.max.z; k++) {
                            glm::vec3 p(i, j, k);
                            float d = point_triangle_distance(p, t.v1, t.v2, t.v3);
                            sdf.insert_or_assign(p, glm::abs(sdf[p]) > glm::abs(d) ? d : sdf[p]);
                        }
                    }
                }

                auto fip=(float)t.v1[0], fjp=(float)t.v1[1], fkp=(float)t.v1[2];
                auto fiq=(float)t.v2[0], fjq=(float)t.v2[1], fkq=(float)t.v2[2];
                auto fir=(float)t.v3[0], fjr=(float)t.v3[1], fkr=(float)t.v3[2];

                // intersection part
                int j0 = glm::ceil(glm::min(fjp,glm::min(fjq,fjr)));
                int j1 = glm::floor(glm::max(fjp,glm::max(fjq,fjr)));
                int k0 = glm::ceil(glm::min(fkp,glm::min(fkq,fkr)));
                int k1 = glm::floor(glm::max(fkp,glm::max(fkq,fkr)));
                for(int k=k0; k<=k1; ++k) {
                    for(int j=j0; j<=j1; ++j){
                        float a, b, c;
//                        if(point_in_triangle_2d(j, k, fjp, fkp, fjq, fkq, fjr, fkr, a, b, c)){
                        if(point_in_triangle_2d(j, k, fjp, fkp, fjq, fkq, fjr, fkr, a, b, c)){
                            float fi=a * t.v1.x + b * t.v1.y + c * t.v1.z; // intersection i coordinate
                            int i_interval = int(glm::ceil(fi)); // intersection is in (i_interval-1,i_interval]
//                            if(i_interval<0) ++intersection_count(0, j, k); // we enlarge the first interval to include everything to the -x direction
                            if (i_interval < 0) {
                                intersections[0][j][k] = intersections[0][j][k] + 1;
                            }
//                            else if(i_interval<ni) ++intersection_count(i_interval,j,k);
                            else {
                                intersections[i_interval][j][k] = intersections[i_interval][j][k] + 1;
                            }
                            // we ignore intersections that are beyond the +x side of the grid
                        }
                    }
                }
            }

            for (int i = (int)minPos.x; i < (int)maxPos.x; i++) {
                for (int j = (int)minPos.y; j < (int)maxPos.y; j++) {
                    int total_count=0;
                    for (int k = (int)minPos.z; k < (int)maxPos.z; k++) {
                        total_count+=intersections[i + int(minPos.x)][j + int(minPos.y)][k + int(minPos.z)];
                        if(total_count%2==1){ // if parity of intersections so far is odd,
                            sdf.insert_or_assign(glm::vec3(i, j, k), -sdf[glm::vec3(i, j, k)]); // we are inside the mesh
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

            // TODO sweeping??
            for (int i = (int)minPos.x; i < (int)maxPos.x; i++) {
                for (int j = (int)minPos.y; j < (int)maxPos.y; j++) {
                    for (int k = (int)minPos.z; k < (int)maxPos.z; k++) {
                        glm::vec3 p(i, j, k);
                        float sd = sdf[p];
                        SDFData d = GetData(p);
                        if (sd < 0.0f) {
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