#version 460 core

layout (location = 0) in vec3 aPos; // from the quad
layout (location = 1) in vec3 iOffset;
layout (location = 2) in vec3 iColor;
layout (location = 3) in float iRadius;

uniform mat4 proj;
uniform mat4 view;

out vec3 vColor;
out float vRadius;
out vec3 vCenter; // viewspace position of the fragment
out vec2 vTexCoord;

void main() {
    vColor = iColor;
    vRadius = iRadius;

    mat4 model = mat4(1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    iOffset.x, iOffset.y, iOffset.z, 1.0);

    mat4 newView = view * model; // GEPE3d inspired
    newView[0][0] = 1;   newView[1][0] = 0;   newView[2][0] = 0;
    newView[0][1] = 0;   newView[1][1] = 1;   newView[2][1] = 0;
    newView[0][2] = 0;   newView[1][2] = 0;   newView[2][2] = 1;

    vCenter = (view * vec4(iOffset, 1.0)).xyz;
    vTexCoord = (aPos.xy) * vRadius;

    gl_Position = proj * newView * (vec4(aPos * iRadius, 1.0));
}