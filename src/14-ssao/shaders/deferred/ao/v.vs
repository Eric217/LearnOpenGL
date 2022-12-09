#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 texCoor;

out vec2 tex_coor;

void main() {
    tex_coor = vec2(texCoor.x, 1 - texCoor.y);
    gl_Position = vec4(pos.x, pos.y, 1, 1);
}
