#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 outTexCoor;

void main() {
    // GL 希望输出的是 clip-space coordinate，然后内部处理裁剪、除以 w、z-test
    gl_Position = projection * view * model * vec4(pos, 1);
    outTexCoor = texCoor;
}
