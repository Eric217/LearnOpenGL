#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoor;

//uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


struct Payload {
    vec2 tex_coor;
    vec3 pos; // view space coor
    vec3 normal; // view space coor
};

out Payload frag;

void main() {
    vec4 vecPos4 = view * vec4(pos, 1);
    
    frag.pos = vecPos4.xyz;
    frag.tex_coor = texCoor;
    frag.normal = (view * vec4(normal, 0)).xyz;

    // GL 希望输出的是 clip-space coordinate，然后内部处理裁剪、除以 w、z-test
    gl_Position = projection * vecPos4;
}
