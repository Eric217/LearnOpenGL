#version 330 core
layout(triangles) in;
layout(line_strip, max_vertices = 2) out;

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

void main() {
    vec3 normal = normalize(cross(vertices[1].raw_pos - vertices[0].raw_pos,
                                  vertices[2].raw_pos - vertices[0].raw_pos));
    vec3 center = vertices[0].raw_pos / 3 + vertices[1].raw_pos / 3 +
                       vertices[2].raw_pos / 3;
    
    gl_Position = projection * view * vec4(center, 1);
    frag.raw_pos = center;
    frag.raw_normal = normal;
    frag.tex_coor = vertices[0].tex_coor / 3 + vertices[1].tex_coor / 3 + vertices[2].tex_coor / 3;
    EmitVertex();
    
    center += explosion * normal;
    gl_Position = projection * view * vec4(center, 1);
    frag.raw_pos = center;
    EmitVertex();
    
    EndPrimitive();
}
 
