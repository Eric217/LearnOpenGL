#version 330 core

layout (triangles) in;
// 1 变 6，分别输出到 cube 六个面
layout (triangle_strip, max_vertices=18) out;

layout(std140) uniform Transforms {
    mat4 matrices[6];
};

// in gl_Vertex
// {
//     vec4  gl_Position;
// } gl_in[];
 
out Payload {
    vec4 pos; // world
} frag;
 
 
void main() {
    for (int i = 0; i < 6; i++) {
        // 指定 gl_Layer 来自动输出到 cubemap 相应的面
        gl_Layer = i;
        for (int j = 0; j < 3; j++) {
            frag.pos = gl_in[j].gl_Position;
            // mvp
            gl_Position = matrices[i] * frag.pos;
            EmitVertex();
        }
        EndPrimitive();    
    } 
}
