#version 330 core

layout(location = 0) in vec3 pos;
//layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoor;

layout(location = 3) in mat4 trans;

// UBO:
// 2 * (4 * 16)
layout(std140) uniform ViewProjection {
    mat4 view;
    mat4 projection;
};

out Payload {
    vec2 tex_coor;
    //vec3 raw_pos; // world space coor
    //vec3 raw_normal; // world space coor
} vertex;

void main() {
    //vertex.raw_pos = pos;
    vertex.tex_coor = texCoor;
    //vertex.raw_normal = normal;

    gl_Position = projection * view * trans * vec4(pos, 1);
}
