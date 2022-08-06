#version 460 core

out vec4 FragColor;

// maybe do that it won't render if out of window
in vec3 vColor;
in float vRadius;
in vec3 vCenter; // don't care
in vec2 vPosOnQuad;

void main() {
    float dist = length(vPosOnQuad);
    if (dist > vRadius) {
        discard;
    }
    FragColor = vec4(vColor * (cos((0.5 / vRadius) * dist)), 1.0f);
}
