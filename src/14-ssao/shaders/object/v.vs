#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoor;
 
// UBO:
// 2 * (4 * 16)
layout(std140) uniform ViewProjection {
    mat4 view;
    mat4 projection;
};
 
out Payload {
    vec2 tex_coor;
    vec4 pos_world;
    vec3 pos; // view space coor
    vec3 normal; // view space coor
} frag;

void main() {
    frag.pos_world = vec4(pos, 1);
    vec4 vecPos4 = view * frag.pos_world;
    
    frag.pos = vecPos4.xyz;
    frag.normal = (view * vec4(normal, 0)).xyz;
    frag.tex_coor = texCoor;

    gl_Position = projection * vecPos4;
}
