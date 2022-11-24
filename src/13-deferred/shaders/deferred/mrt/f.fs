#version 330 core

layout(location = 0) out vec3 pos;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec4 color;

in Payload {
    vec3 pos_world;
    vec3 normal_view;
    vec2 tex_coor;
} frag;

struct Material {
    bool use_texture_diffuse0;
    sampler2D texture_diffuse0;

    bool use_texture_specular0;
    sampler2D texture_specular0;
};

uniform Material material;

void main() {
    pos = frag.pos_world;
    normal = frag.normal_view;
    color = vec4(0);
    if (material.use_texture_diffuse0) {
        color = texture(material.texture_diffuse0, frag.tex_coor);
    }
    color.a = 0;
    if (material.use_texture_specular0) {
        color.a = texture(material.texture_specular0, frag.tex_coor).r;
    } 
}
