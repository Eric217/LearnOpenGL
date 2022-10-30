#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoor;

// UBO:
// uniform block layout
// 占 16 * 4
layout(std140) uniform View {
    mat4 view;
};

// 占 16 * 4
layout(std140) uniform Projection {
    mat4 projection;
};

out Payload {
    vec2 tex_coor;
    vec3 raw_pos; // world space coor
    vec3 raw_normal; // world space coor
} vertex;

void main() {
    vec4 vecPos4 = view * vec4(pos, 1);
    
    vertex.raw_pos = pos;
    vertex.tex_coor = texCoor;
    vertex.raw_normal = normal;

    gl_Position = projection * vecPos4;
}
