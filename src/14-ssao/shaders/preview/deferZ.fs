#version 330 core

struct Material {
    sampler2D texture_diffuse0;
};

uniform Material material;

in vec2 tex_coor;

out vec4 color;

void main() {
    float gray = texture(material.texture_diffuse0, tex_coor).a;
    gray /= 120; // far plane
    color = vec4(gray);
}

