#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoor;

//uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


struct Payload {
    vec2 tex_coor;
    vec3 raw_pos; // world space coor
    vec3 raw_normal; // world space coor
};

out Payload frag;

void main() {
    vec4 vecPos4 = view * vec4(pos, 1);
    
    frag.raw_pos = pos;
    frag.tex_coor = texCoor;
    frag.raw_normal = normal;

    gl_Position = projection * vecPos4;
}
