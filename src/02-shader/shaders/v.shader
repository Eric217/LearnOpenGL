#version 330 core

uniform float customX;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 dataColor;

out vec3 outColor;

void main() {
    gl_Position = vec4(pos.x + customX, -pos.y, pos.z, 1);
    outColor = gl_Position.xyz + dataColor * 0.1;
}
