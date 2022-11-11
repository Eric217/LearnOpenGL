#version 330 core

layout(location = 0) in vec3 pos;

void main() {
    // 好像裁剪、除以 w 是在几何阶段之后？
    gl_Position = vec4(pos, 1);
}

