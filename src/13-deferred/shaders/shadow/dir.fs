#version 330 core

struct Payload {
    vec3 pos; // ndc space pos, [-1, 1]
};

in Payload frag;
 
void main() {
    gl_FragDepth = frag.pos.z * 0.5 + 0.5;
}
