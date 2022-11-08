#version 330 core

struct Material {
    bool use_texture_diffuse0;
    sampler2D texture_diffuse0;

    bool use_texture_specular0;
    sampler2D texture_specular0;
};

uniform Material material;

struct DirLight {
    vec3 direction; // world space coor
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    mat4 trans;
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

#define NR_POINT_LIGHTS 0
#define NR_DIR_LIGHTS 1
#define NR_SPOT_LIGHTS 0

// uniform DirLight dirLights[NR_DIR_LIGHTS];
//uniform PointLight pointLights[NR_POINT_LIGHTS];
//uniform SpotLight spotLights[NR_SPOT_LIGHTS];

// UBO: 4 * vec3 + mat4 = NR * 8 * 16
layout(std140) uniform DirLights {
    DirLight dirLights[NR_DIR_LIGHTS];
};

struct LightMap {
    sampler2D texture_dirLight0;
    bool use_texture_dirLight0;
};
uniform LightMap lights;

// UBO:
// 2 * (4 * 16)
layout(std140) uniform ViewProjection {
    mat4 view;
    mat4 projection;
};
  
in Payload {
    vec2 tex_coor;
    vec4 pos_world;
    vec3 pos; // view space coor
    vec3 normal; // view space coor
} frag;
 

out vec4 color;


vec3 calcPointLight(PointLight light);
vec3 calcSpotLight(SpotLight light);
vec3 calcDirectionalLight(DirLight light);

void main() {
    vec3 result = vec3(0);
    for (int i = 0; i < NR_DIR_LIGHTS; i++) {
        result += calcDirectionalLight(dirLights[i]);
    }
    // for (int i = 0; i < NR_POINT_LIGHTS; i++) {
    //     result += calcPointLight(pointLights[i]);
    // }
    // for (int i = 0; i < NR_SPOT_LIGHTS; i++) {
    //     result += calcSpotLight(spotLights[i]);
    // }
    
    color = vec4(result, 1);
    //color = vec4(pow(result, vec3(1/2.2)), 1);
}
 
bool inDirLightShadow(mat4 trans, vec3 toLightN) {
    if (!lights.use_texture_dirLight0) {
        return false;
    }
    vec4 lightSpacePos = trans * frag.pos_world;
    /// get [-1, 1] pos
    vec3 ndcPos = lightSpacePos.xyz / lightSpacePos.w;
    /// [0-1] pos
    ndcPos = ndcPos * 0.5 + 0.5;
    float depth = texture(lights.texture_dirLight0, ndcPos.xy).r;
    // 用系数 0.001，最小 0.0002 就行，但为了表演 Peter Panning 调大点
    float bias = max(0.0002, (1 - abs(dot(frag.normal, toLightN))) * 0.005);
    return ndcPos.z > depth + bias;
}

/// 其他参数：Payload frag, Material material, MVP
vec3 calcPointLight(PointLight light) {
    vec3 tex_color = vec3(0);
    if (material.use_texture_diffuse0) {
        tex_color = texture(material.texture_diffuse0, frag.tex_coor).xyz;
    }
    
    // eye space coor
    vec3 toLight = (view * vec4(light.position, 1) - vec4(frag.pos, 1)).xyz;
    vec3 toLightN = normalize(toLight);
    vec3 toEye = normalize(-frag.pos);

    float cos = max(0, dot(toLightN, frag.normal));
    float distance = length(toLight);
    float attenuation = 1.0
        / (light.k0 + light.k1 * distance + light.k2 * distance * distance);

    vec3 amb = light.ambient * tex_color * 0.0006;
    vec3 diff = cos * light.diffuse * tex_color;
    // 高光
    vec3 halfV = normalize((toLightN) + toEye);
    
    tex_color = vec3(0);
    if (material.use_texture_specular0) {
        tex_color = texture(material.texture_specular0, frag.tex_coor).xyz;
    }
    // 使用半程而不是反射（Blinn 区别）
    float cos2 = dot(halfV, frag.normal);
    vec3 spec;
    if (cos > 0 && cos2 > 0 && dot(toEye, frag.normal) > 0) {
        spec = pow(cos2, 150) * light.specular * tex_color;
    } else {
        spec = vec3(0);
    }
    return (amb + diff + spec) * attenuation;
}

/// 其他参数：Payload frag, Material material, MVP
vec3 calcSpotLight(SpotLight light) {
    vec3 tex_color = vec3(0);
    if (material.use_texture_diffuse0) {
        tex_color = texture(material.texture_diffuse0, frag.tex_coor).xyz;
    }
    // eye space coor
    vec3 toLightI = (vec4(frag.pos, 1) - view * vec4(light.position, 1)).xyz;
    vec3 toLightN = normalize(-toLightI);
    vec3 toEye = normalize(-frag.pos);
    vec3 lightDir = normalize((view * vec4(light.direction, 0)).xyz);

    // 计算聚光内外
    float cos = dot(normalize(toLightI), lightDir);
    float intensity = (cos - light.outerCutOff) / (light.cutOff - light.outerCutOff);
    intensity = clamp(intensity, 0, 1);

    float distance = length(toLightI);
    float attenuation = 1.0
        / (light.k0 + light.k1 * distance + light.k2 * distance * distance);
    
    vec3 amb = light.ambient * tex_color * 0.0005;

    cos = max(0, dot(frag.normal, toLightN));
    vec3 diff = cos * light.diffuse * tex_color * intensity;
    // 高光
    vec3 halfV = normalize((toLightN) + toEye);
    // 使用半程而不是反射（Blinn 区别）
    float cos2 = dot(halfV, frag.normal);
    vec3 spec;
    
    
    if (intensity > 0 && cos > 0 && cos2 > 0 && dot(toEye, frag.normal) > 0) {
        tex_color = vec3(0);
        if (material.use_texture_specular0) {
            tex_color = texture(material.texture_specular0, frag.tex_coor).xyz;
        }
        spec = pow(cos2, 150) * light.specular * tex_color * intensity;
    } else {
        spec = vec3(0);
    }
    return (amb + diff + spec) * attenuation;
}

/// 其他参数：Payload frag, Material material, MVP
vec3 calcDirectionalLight(DirLight light) {
    vec3 tex_color = vec3(0);
    if (material.use_texture_diffuse0) {
        tex_color = texture(material.texture_diffuse0, frag.tex_coor).xyz;
    }
    vec3 toLightN = normalize(-vec3(view * vec4(light.direction, 0)));
    vec3 toEye = normalize(-frag.pos);

    float cos = max(0, dot(toLightN, frag.normal));

    vec3 amb = light.ambient * tex_color * 0.015;
    vec3 diff = cos * light.diffuse * tex_color;
    // 高光
    vec3 halfV = normalize((toLightN) + toEye);
    
    tex_color = vec3(0);
    if (material.use_texture_specular0) {
        tex_color = texture(material.texture_specular0, frag.tex_coor).xyz;
    }
    // 使用半程而不是反射（Blinn 区别）
    float cos2 = dot(halfV, frag.normal);
    vec3 spec;
    if (cos > 0 && cos2 > 0 && dot(toEye, frag.normal) > 0) {
        spec = pow(cos2, 150) * light.specular * tex_color;
    } else {
        spec = vec3(0);
    }
    if (inDirLightShadow(light.trans, toLightN)) {
        return amb;
    }
    return amb + diff + spec;
}
