#version 330 core

out vec4 color;

struct Material {
    bool use_texture_diffuse0;
    sampler2D texture_diffuse0;
    // float diffuseValue; 不做那么完整了，没有纹理就用白色

    bool use_texture_specular0;
    sampler2D texture_specular0;
};

uniform Material material;

struct Payload {
    vec2 tex_coor;
    vec3 pos; // view space coor
    vec3 normal; // view space coor
};

in Payload frag;

void main() {
    vec4 texel = texture(material.texture_diffuse0, frag.tex_coor);
    
    color = texel;
}
