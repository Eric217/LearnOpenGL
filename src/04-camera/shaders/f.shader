#version 330 core

in vec2 outTexCoor;

out vec4 color;

uniform sampler2D image0;
uniform sampler2D image1;

uniform float mixLevel;

void main() {
    color = mix(texture(image0, outTexCoor), texture(image1, outTexCoor),
        mixLevel);
}
