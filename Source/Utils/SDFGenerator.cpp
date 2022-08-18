#include "SDFGenerator.h"

namespace SDFGenerator {
    SDFData Cube(glm::vec3 p, glm::vec3 b, float step) {
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

    SDFData Ball(glm::vec3 p, float s, float step) {
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

} // SDFGenerator