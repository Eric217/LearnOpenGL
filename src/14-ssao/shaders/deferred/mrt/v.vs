#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoor;
 
// UBO:
// 2 * (4 * 16)
layout(std140) uniform ViewProjection {
    mat4 view;
    mat4 projection;
};

out Payload {
    vec3 pos_world;
    vec3 normal_view;
    vec2 tex_coor;
    float z_view;
} frag;

void main() {
    frag.pos_world = pos; 
    frag.normal_view = (view * vec4(normal, 0)).xyz;
    frag.tex_coor = texCoor;
    vec4 _pos_view = view * vec4(pos, 1); 
    frag.z_view = _pos_view.z;
    
    gl_Position = projection * view * vec4(pos, 1);
}
