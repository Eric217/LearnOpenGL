#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoor;

// UBO:
// 16 * 4
layout(std140) uniform View {
    mat4 view;
};

//uniform  mat4 view;
// 16 * 4
layout(std140) uniform Projection {
    mat4 projection;
};
//uniform mat4 projection;
out Payload {
    vec2 tex_coor;
    vec3 raw_pos; // world space coor
    vec3 raw_normal; // world space coor
} frag;

void main() {
    vec4 vecPos4 = view * vec4(pos, 1);
    
    frag.tex_coor = texCoor;
    frag.raw_pos = pos;
    frag.raw_normal = normal;

    gl_Position = projection * vecPos4;
}
