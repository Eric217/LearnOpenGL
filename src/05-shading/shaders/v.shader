#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 normalTransform;

out vec2 outTexCoor;
out vec3 normal_f;
out vec3 vecPos;

void main() {
    // GL 希望输出的是 clip-space coordinate，然后内部处理裁剪、除以 w、z-test
    vec4 vecPos4 = view * model * vec4(pos, 1);
    vecPos = vecPos4.xyz;
    gl_Position = projection * vecPos4;
    outTexCoor = texCoor;
    normal_f = normalize((normalTransform * vec4(normal, 0)).xyz);
}
