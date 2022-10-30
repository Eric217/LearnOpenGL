#version 330 core

// UBO: 16 * 1
layout(std140) uniform Uniforms {
    vec3 cameraPos;
};

struct Material {
    bool use_texture_diffuse0;
    sampler2D texture_diffuse0;

    bool use_texture_specular0;
    sampler2D texture_specular0;
    
    bool use_texture_cubemap2d0;
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
 
in Payload {
    vec2 tex_coor;
    vec3 raw_pos; // world space coor
    vec3 raw_normal; // world space coor
} frag;

vec4 getColor(vec3 tex_coor);

out vec4 color;

void main() {
    // 用 specular 做 reflection map 添加一个环境映射

    // 1 diffuse 做底色
    vec4 diffuseColor = texture(material.texture_diffuse0, frag.tex_coor);

    // 2 读出 specular 值，做环境映射系数
    vec4 specularColor = texture(material.texture_specular0, frag.tex_coor);
    float shininess = (specularColor.r + specularColor.g + specularColor.b) / 3;
    // 3 反射的环境颜色
    vec3 outdir = reflect(frag.raw_pos - cameraPos, normalize(frag.raw_normal));
    vec4 texel = getColor(outdir);
    // 4 加权
    color = shininess * texel + (1 - shininess) * diffuseColor;
}

vec4 getColor(vec3 tex_coor) {
    if (material.use_texture_cubemap0) {
        return texture(material.texture_cubemap0, tex_coor);
    }
    // 找到长度最大的分量 确定要采样的纹理
    vec3 coorA = abs(tex_coor);
    float p = tex_coor.p;
    float s = tex_coor.s;
    float t = tex_coor.t;

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
    return texel;
}
