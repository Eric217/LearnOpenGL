#version 330 core

struct Material {
    bool use_texture_diffuse0;
    sampler2D texture_diffuse0;
};

uniform Material material;
uniform bool usingPing;

struct Payload {
    vec2 tex_coor;
};

in Payload frag;

out vec4 color;

// kernel 这里是直径，目前用的很小。。
#define KernelSize 9

float weights[KernelSize] = float[]
(
    0.016216, 0.054054, 0.1216216, 0.1945946,
    0.227027,
    0.1945946, 0.1216216, 0.054054, 0.016216
);

void main() {
    if (!material.use_texture_diffuse0) {
        color = vec4(1, 0, 0, 1);
        return;
    }
    vec2 texSize = 1.0 / textureSize(material.texture_diffuse0, 0);
    float unit_w = texSize.x;
    float unit_h = texSize.y;

    // 我做的模型有问题，需要反转纹理坐标 y
    vec2 center = vec2(frag.tex_coor.x, 1 - frag.tex_coor.y);
    
    if (usingPing) { // row conv
        float base_x = center.x - float(int(KernelSize / 2)) * unit_w;
        for (int i = 0; i < KernelSize; i++) {
            color += weights[i] * texture(material.texture_diffuse0, vec2(base_x, center.y));
            base_x += unit_w;
        }
    } else {
        float base_y = center.y - int(KernelSize / 2) * unit_h;
        for (int i = 0; i < KernelSize; i++) {
            color += weights[i] * texture(material.texture_diffuse0, vec2(center.x, base_y));
            base_y += unit_h;
        }
    }

}

