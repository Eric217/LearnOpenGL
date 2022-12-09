#version 330 core

struct Material {
    sampler2D texture_diffuse0;
    bool use_texture_diffuse0;
};

uniform Material material;

in vec2 tex_coor;

out vec4 color;

void main() {
    vec2 _size = textureSize(material.texture_diffuse0, 0);
   color = texture(material.texture_diffuse0, tex_coor * _size / 4);
    // if (gl_FragCoord.x > 300) {
    //     color = texture(material.texture_diffuse0, 3 / _size);
    // } else {
    //     color = texture(material.texture_diffuse0, 4 / _size);
    // }
    color = (color + 1) * 0.5; 
}

