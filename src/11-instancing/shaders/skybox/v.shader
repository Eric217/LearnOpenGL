#version 330 core

layout(location = 0) in vec3 pos;

// UBO:
// 2 * (4 * 16)
layout(std140) uniform ViewProjection {
    mat4 view;
    mat4 projection;
};
  
struct Payload {
    vec3 tex_coor; // pos without translation
};

out Payload frag;

void main() {
    frag.tex_coor = pos;

    mat3 view3 = mat3(view);
    gl_Position = projection * vec4(view3 * pos, 1);
    
    gl_Position.z = gl_Position.w;
}
