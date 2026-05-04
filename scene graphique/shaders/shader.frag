#version 330 core
in vec3 fragPos;
in vec3 fragNormal;
in vec2 fragUV;

uniform sampler2D uTexture;
uniform vec3  uColor;
uniform bool  uUseTexture;
uniform vec3  uLightPos;
uniform vec3  uViewPos;
uniform bool  uEmissive;
uniform float uLightIntensity;
uniform float uAmbientIntensity;

out vec4 FragColor;

void main() {
    vec4 sample   = uUseTexture ? texture(uTexture, fragUV) : vec4(uColor, 1.0);
    vec3 texColor = sample.rgb;
    float alpha   = sample.a;

    if (uEmissive) {
        FragColor = vec4(texColor, alpha);
        return;
    }

    vec3 N = normalize(fragNormal);
    vec3 L = normalize(uLightPos - fragPos);
    vec3 V = normalize(uViewPos  - fragPos);
    vec3 R = reflect(-L, N);

    float ambient  = uAmbientIntensity;
    float diffuse  = max(dot(N, L), 0.0) * uLightIntensity;
    float specular = pow(max(dot(R, V), 0.0), 32.0) * 0.4 * uLightIntensity;

    vec3 col = texColor * (ambient + diffuse) + vec3(specular);
    col = col / (col + vec3(1.0));

    FragColor = vec4(col, alpha);
}
