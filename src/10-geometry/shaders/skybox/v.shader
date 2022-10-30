#version 330 core

layout(location = 0) in vec3 pos;

// UBO: 16 * 4
layout(std140) uniform Projection {
    mat4 projection;
};

uniform mat3 view;

struct Payload {
    vec3 tex_coor; // pos without translation
};

out Payload frag;

void main() {
    frag.tex_coor = pos;

    gl_Position = projection * vec4(view * pos, 1);
    gl_Position.z = gl_Position.w;
}
