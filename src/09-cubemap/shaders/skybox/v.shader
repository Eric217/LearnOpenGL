#version 330 core

layout(location = 0) in vec3 pos;

uniform mat3 view;
uniform mat4 projection;


struct Payload {
    vec3 tex_coor; // pos without translation
};

out Payload frag;

void main() {
    frag.tex_coor = pos;

    gl_Position = projection * vec4(view * pos, 1);
    gl_Position.z = gl_Position.w;
}
