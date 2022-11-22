#version 330 core
layout(location = 0) out vec4 color;
layout(location = 1) out vec4 color1;

uniform vec3 lightColor;


void main() {
    color = vec4(lightColor, 1);
    color1 = color;
}
