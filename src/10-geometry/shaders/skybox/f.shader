#version 330 core

struct Material {
    sampler2D texture_cubemap2d0;
    sampler2D texture_cubemap2d1;
    sampler2D texture_cubemap2d2;
    sampler2D texture_cubemap2d3;
    sampler2D texture_cubemap2d4;
    sampler2D texture_cubemap2d5;
    
    bool use_texture_cubemap0;
    samplerCube texture_cubemap0;
};

uniform Material material;
 
struct Payload {
    vec3 tex_coor; // pos without translation
};

in Payload frag;

out vec4 color;
 
void main() {
    if (material.use_texture_cubemap0) {
        color = texture(material.texture_cubemap0, frag.tex_coor);
        return;
    }
    // 找到长度最大的分量 确定要采样的纹理
    vec3 coorA = abs(frag.tex_coor);
    float p = frag.tex_coor.p;
    float s = frag.tex_coor.s;
    float t = frag.tex_coor.t;

    float m;
    vec4 texel = vec4(0);
    // x 轴先到达
    if (coorA.s >= coorA.t && coorA.s >= coorA.p) {
        m = coorA.s;
        if (s > 0) { // t p sample +x
            texel = texture(material.texture_cubemap2d0, vec2(1-(p/m + 1)*0.5, (t/m + 1) * 0.5));
        } else {  // t p sample -x
            texel = texture(material.texture_cubemap2d1, vec2((p/m + 1)*0.5, (t/m+1)*0.5));
        }
    }
    // y 轴先到达
    else if (coorA.t >= coorA.s && coorA.t >= coorA.p) {
        m = coorA.t;
        if (t > 0) { // sample +y
            texel = texture(material.texture_cubemap2d2, vec2((s/m + 1)*0.5, 1-(p/m + 1) * 0.5));
        } else {  // sample -y
            texel = texture(material.texture_cubemap2d3, vec2((s/m + 1)*0.5,  (p/m+1)*0.5));
        }
    }
    // z 轴先到达
    else if (coorA.p >= coorA.t && coorA.p >= coorA.s) {
        m = coorA.p;
        if (p > 0) { // sample +z
            texel = texture(material.texture_cubemap2d4, vec2((s/m + 1)*0.5,  (t/m + 1) * 0.5));
        } else {  // sample -z
            texel = texture(material.texture_cubemap2d5, vec2(1-(s/m + 1)*0.5,  (t/m+1)*0.5));
        }
    }
   
    color = texel;
}
