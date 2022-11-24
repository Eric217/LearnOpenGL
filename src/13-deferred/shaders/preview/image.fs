#version 330 core

struct Material {
    sampler2D texture_diffuse0;
};

uniform Material material;

in vec2 tex_coor;

out vec4 color;

void main() {
    color = texture(material.texture_diffuse0, tex_coor);
}

