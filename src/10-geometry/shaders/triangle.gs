#version 330 core
layout(triangles) in;
layout(line_strip, max_vertices = 4) out;

// in gl_Vertex
// {
//     vec4  gl_Position;
//     float gl_PointSize;
//     float gl_ClipDistance[];
// } gl_in[];

in Payload {
    vec2 tex_coor;
    vec3 raw_pos; // world space coor
    vec3 raw_normal; // world space coor
} vertices[];

out Payload {
    vec2 tex_coor;
    vec3 raw_pos; // world space coor
    vec3 raw_normal; // world space coor
} frag;

void doVertex(int index) {
    gl_Position = gl_in[index].gl_Position;
    frag.tex_coor = vertices[index].tex_coor;
    frag.raw_pos = vertices[index].raw_pos;
    frag.raw_normal = vertices[index].raw_normal;
    EmitVertex();
}

void main() {
    doVertex(0);
    doVertex(1);
    doVertex(2);
    doVertex(0);

    EndPrimitive();
}
 
