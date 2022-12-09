#version 330 core

layout(location = 0) in vec3 pos;

uniform mat4 vpMatrix;
  
struct Payload {
    vec3 pos; // ndc space pos, [-1, 1]
};

out Payload frag;

void main() {
    gl_Position = vpMatrix * vec4(pos, 1);
    frag.pos = gl_Position.xyz / gl_Position.w;
}
