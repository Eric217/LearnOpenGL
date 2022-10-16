#version 330 core

layout(location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 vecPos4 = view * model * vec4(pos, 1);
    gl_Position = projection * vecPos4;
}
