#version 330 core

struct LightMap {
    sampler2D texture_dirLight0;
    bool use_texture_dirLight0;
};

uniform LightMap lights;

out vec4 color;

void main() {
    if (lights.use_texture_dirLight0) { 
        float v = texture(lights.texture_dirLight0, vec2(gl_FragCoord.x / 1400.f, gl_FragCoord.  y / 1120.f)).r;
        color = vec4(v, v, v, 1);
    } else {
        color = vec4(1, 0, 0, 1);
    }
}

