#version 330 core

struct LightMap {
    bool use_texture_pointLight0;
    samplerCube texture_pointLight0;
};

uniform LightMap lights;

in vec3 raw_pos;

out vec4 color;

void main() {
    if (lights.use_texture_pointLight0) {
        float depth = texture(lights.texture_pointLight0, raw_pos).r;
        color = vec4(vec3(depth), 1);
    } else {
        color = vec4(1, 0, 0, 1);
    }
}

