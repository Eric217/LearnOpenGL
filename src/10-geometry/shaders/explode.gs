#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

// in gl_Vertex
// {
//     vec4  gl_Position;
//     float gl_PointSize;
//     float gl_ClipDistance[];
// } gl_in[];

layout(std140) uniform View {
    mat4 view;
};
layout(std140) uniform Projection {
    mat4 projection;
};
layout(std140) uniform Explosion {
    float explosion;
};

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

void doVertex(int index, vec3 normal) { 
    vec4 pos = vec4(vertices[index].raw_pos + normal, 1);
    frag.raw_pos = pos.xyz;
    frag.raw_normal = vertices[index].raw_normal;
    frag.tex_coor = vertices[index].tex_coor;
    gl_Position = projection * view * pos; 
    EmitVertex();
}


void main() {
    vec3 normal = cross(vertices[1].raw_pos - vertices[0].raw_pos, 
                        vertices[2].raw_pos - vertices[0].raw_pos);
    normal = explosion * normalize(normal);
     
    doVertex(0, normal);
    doVertex(1, normal);
    doVertex(2, normal); 

    EndPrimitive();
}
 
