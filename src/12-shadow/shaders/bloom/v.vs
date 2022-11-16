#version 330 core
layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 texCoor;

struct Payload {
    vec2 tex_coor;
};

out Payload frag;

void main() {
    frag.tex_coor = texCoor;

    gl_Position = vec4(pos, 1);
  
}
