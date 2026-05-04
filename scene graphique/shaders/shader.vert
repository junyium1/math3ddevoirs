#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

out vec3 fragPos;
out vec3 fragNormal;
out vec2 fragUV;

void main() {
    vec4 world = uModel * vec4(aPos, 1.0);
    fragPos    = world.xyz;
    fragNormal = mat3(transpose(inverse(uModel))) * aNormal;
    fragUV     = aUV;
    gl_Position = uProj * uView * world;
}
