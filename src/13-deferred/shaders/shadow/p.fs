#version 330 core

in Payload {
    vec4 pos; // world
} frag;
 
uniform float farPlane;
uniform vec4 cameraPos;

void main() {
    gl_FragDepth = distance(frag.pos, cameraPos) / farPlane;
}
