#version 460 core

out vec4 FragColor;

// maybe do that it won't render if out of window
in vec3 vColor;
in float vRadius;
in vec3 vCenter; // don't care
in vec2 vTexCoord;

uniform mat4 view;
uniform mat4 proj;

float near = 0.001;
float far = 10000.0;

float linearizeDepth(float depth){
    return (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));
}

void main() {
    vec3 normal;
    normal.xy = vTexCoord * 2.0 - 1.0;
    float r2 = dot(normal.xy, normal.xy);
    if (length(vTexCoord) > vRadius) {
        discard;
    }
    normal.z = sqrt(1.0 - r2);

    vec4 pixelPos = vec4(vCenter + normal * vRadius, 1.0);
    vec4 clipSpacePos = proj * pixelPos;
    float fragDepth = clipSpacePos.z / clipSpacePos.w;

    vec3 lightPos = vec3(0.0, 0.0, 0.0);
    vec3 lightDir = normalize(lightPos - vCenter);
    float diffuse = max(0.0, dot(normal, lightDir));
    vec3 color = vColor * cos((0.5 / vRadius) * length(vTexCoord));
//    vec3 color = vec3(linearizeDepth(fragDepth));
    FragColor = vec4(color, 1.0);
}
