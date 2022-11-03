#version 330 core

out vec4 color;

struct Material {
    sampler2D texture_diffuse0;
};

uniform Material material;

in Payload {
    vec2 tex_coor;
} frag;

void main() {
    // 只做一个漫反射
    color = texture(material.texture_diffuse0, frag.tex_coor);
}
 
