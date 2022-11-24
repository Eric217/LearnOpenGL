#version 330 core

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 color1;

struct Material {
    sampler2D texture_diffuse0;
    sampler2D texture_diffuse1;
};

uniform Material material;

in vec2 tex_coor;

void main() {
    color = texture(material.texture_diffuse0, tex_coor);
    color1 = texture(material.texture_diffuse1, tex_coor);
}

