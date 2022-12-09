#version 330 core

layout(location = 0) in vec3 pos;

// UBO:
// 2 * (4 * 16)
layout(std140) uniform ViewProjection {
    mat4 view;
    mat4 projection;
};

void main() {
    gl_Position = projection * view * vec4(pos, 1);
}
