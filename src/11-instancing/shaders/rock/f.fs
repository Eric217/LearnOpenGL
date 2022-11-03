#version 330 core

struct Material {
    bool use_texture_diffuse0;
    sampler2D texture_diffuse0;

  //  bool use_texture_specular0;
  //  sampler2D texture_specular0;
};

uniform Material material;
 
in Payload {
    vec2 tex_coor;
    //vec3 raw_pos; // world space coor
    //vec3 raw_normal; // world space coor
} frag;


out vec4 color;

void main() {
    color = texture(material.texture_diffuse0, frag.tex_coor);
}
 
