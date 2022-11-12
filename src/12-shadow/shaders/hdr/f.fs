#version 330 core

struct Material {
    bool use_texture_diffuse0;
    sampler2D texture_diffuse0;
};

uniform Material material;

layout(std140) uniform Context {
    int context_screen_w;
    int context_screen_h;
    float context_exposure;
    // float slot;

    // vec4 slot1;
};

out vec4 color;

vec3 hdr_to_ldr(vec3 texel, bool mask) {
    if (!mask || context_exposure <= 0) {
        return texel;
    }
    return vec3(1) - exp(-context_exposure * texel);
}

void main() {
    if (!material.use_texture_diffuse0) {
        color = vec4(1, 0, 0, 1);
        return;
    }
    vec2 coor = vec2(gl_FragCoord.x / context_screen_w, 
                     gl_FragCoord.y / context_screen_h);
    vec4 texel = texture(material.texture_diffuse0, coor);

    color.xyz = hdr_to_ldr(texel.xyz, coor.x < 0.5);
}

