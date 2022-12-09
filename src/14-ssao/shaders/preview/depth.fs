#version 330 core

struct Material {
    sampler2D texture_diffuse0;
};

uniform Material material;

in vec2 tex_coor;

out vec4 color;

void main() {
    float gray = 1 - texture(material.texture_diffuse0, tex_coor).r;
    color = vec4(gray);
}

