#version 330 core

struct Material {
    bool use_texture_diffuse0;
    sampler2D texture_diffuse0;
    // float diffuseValue; 不做那么完整了，没有纹理就用白色

    bool use_texture_specular0;
    sampler2D texture_specular0;
    
    sampler2D texture_diffuse1;
    sampler2D texture_diffuse2;
    sampler2D texture_diffuse3;
    sampler2D texture_diffuse4;
    sampler2D texture_diffuse5;
    sampler2D texture_diffuse5;

    bool use_texture_cubemap0;
    samplerCube texture_cubemap0;
};

uniform Material material;
uniform Material material;

struct DirLight {
    vec3 direction; // world space coor
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position; // world space coor
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float k0;
    float k1;
    float k2;
};

struct SpotLight {
    vec3 position; // world space coor
    vec3 direction; // world space coor

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float cutOff;
    float outerCutOff;

    float k0;
    float k1;
    float k2;
};

#define NR_POINT_LIGHTS 1
#define NR_DIR_LIGHTS 0
#define NR_SPOT_LIGHTS 0

//uniform DirLight dirLights[NR_DIR_LIGHTS];
uniform PointLight pointLights[NR_POINT_LIGHTS];
//uniform SpotLight spotLights[NR_SPOT_LIGHTS];


struct Payload {
    vec2 tex_coor;
    vec3 pos; // view space coor
    vec3 normal; // view space coor
};

in Payload frag;

// 其他
uniform mat4 view; // vertex shader 的 view matrix

out vec4 color;


vec4 calcPointLight(PointLight light);
vec4 calcSpotLight(SpotLight light);
vec4 calcDirectionalLight(DirLight light);

void main() {
    vec4 result = vec4(0);
   
    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
        result += calcPointLight(pointLights[i]);
    }
   
    color = result;
}

/// 其他参数：Payload frag, Material material, MVP
vec4 calcPointLight(PointLight light) {
    vec4 tex_color = vec4(1, 1, 1, 1);
    if (material.use_texture_diffuse0) {
        tex_color = texture(material.texture_diffuse0, frag.tex_coor);
    }
    vec3 tex_color3 = tex_color.rgb;

    // eye space coor
    vec3 toLight = (view * vec4(light.position, 1) - vec4(frag.pos, 1)).xyz;
    vec3 toLightN = normalize(toLight); 
    vec3 toEye = normalize(-frag.pos);

    float cos = max(0, dot(toLightN, frag.normal));
    float distance = length(toLight);
    float attenuation = 1.0 
        / (light.k0 + light.k1 * distance + light.k2 * distance * distance);
        
    vec3 amb = light.ambient * tex_color3 * 0.5;
    vec3 diff = cos * light.diffuse * tex_color3;
    // 高光
    vec3 halfV = normalize((toLightN) + toEye);
    
    tex_color3 = vec3(1, 1, 1);
    if (material.use_texture_specular0) {
        tex_color3 = texture(material.texture_specular0, frag.tex_coor).xyz;
    }
    // 使用半程而不是反射（Blinn 区别）
    float cos2 = dot(halfV, frag.normal);
    vec3 spec;
    float shininess = 150;
    if (cos > 0 && cos2 > 0 && dot(toEye, frag.normal) > 0) {
        spec = pow(cos2, shininess) * light.specular * tex_color3;
    } else {
        spec = vec3(0);
    }
    return vec4((amb + diff + spec) * attenuation, tex_color.a);
}
