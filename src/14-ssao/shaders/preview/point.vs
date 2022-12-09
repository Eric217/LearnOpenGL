#version 330 core

layout(location = 0) in vec3 pos;

// UBO:
// 2 * (4 * 16)
layout(std140) uniform ViewProjection {
    mat4 view;
    mat4 projection;
};
  
out vec3 raw_pos;

void main() {
    raw_pos = pos;
    gl_Position = projection * mat4(mat3(view)) * vec4(pos, 1);
}
