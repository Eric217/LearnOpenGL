#version 330 core

struct Material {
    bool use_texture_diffuse0;
    sampler2D texture_diffuse0;
};

struct Bloom {
    bool use_texture_bloom0;
    sampler2D texture_bloom0;
};

uniform Bloom lights;
uniform Material material;
uniform bool hdrMask; // true 时开启 HDR 开关对比

layout(std140) uniform Context {
    int context_screen_w;
    int context_screen_h;
    float context_exposure;
    float context_far_plane_ps;

    // vec4 slot1;
};

out vec4 color;

vec3 hdr_to_ldr(vec3 texel) {
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

    if (lights.use_texture_bloom0) {
        // 需要设置 threshold，但是我没想出来好的办法？？？
        // 现在暖光灯就已经颜色失真了。。
        texel += texture(lights.texture_bloom0, coor);
    }
    bool useHDR = context_exposure > 0;
    if (!useHDR) {
        color = texel;
        return;
    }
    if (coor.x > 0.5 && hdrMask) {
        color = texel;
        return;
    }
    color.xyz = hdr_to_ldr(texel.xyz);
}

