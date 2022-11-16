#version 330 core
layout(location = 0) out vec4 color;
layout(location = 1) out vec4 color1;

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

/// 5 * vec3 的大小，手动对齐到 2 * mat4
struct PointLight {
    vec3 position; // world space coor
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float k0;
    float k1;
    float k2;

    vec3 slot0;
    vec3 slot1;
    vec3 slot2;
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
#define NR_DIR_LIGHTS 1
#define NR_SPOT_LIGHTS 0

// UBO: 3 * mat4 = NR * 3 * 64
layout(std140) uniform PointLights {
    PointLight pointLights[NR_POINT_LIGHTS];
};

// UBO: 4 * vec3 + mat4 = NR * 8 * 16
layout(std140) uniform DirLights {
    DirLight dirLights[NR_DIR_LIGHTS];
};

struct LightMap {
    sampler2D texture_dirLight0;
    bool use_texture_dirLight0;

    samplerCube texture_pointLight0;
    bool use_texture_pointLight0;
};

uniform LightMap lights;

uniform float farPlane;

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
 

vec3 calcPointLight(PointLight light);
// vec3 calcSpotLight(SpotLight light);
vec3 calcDirectionalLight(DirLight light);

void main() {
    vec3 result = vec3(0);
    for (int i = 0; i < NR_DIR_LIGHTS; i++) {
        result += calcDirectionalLight(dirLights[i]);
    }
    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
        result += calcPointLight(pointLights[i]);
    }
    // for (int i = 0; i < NR_SPOT_LIGHTS; i++) {
    //     result += calcSpotLight(spotLights[i]);
    // }
    color = vec4(result, 1);
}

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

/// 完全在阴影中返回 1
float pointLightShadow(vec3 toLightN) {
    if (!lights.use_texture_pointLight0) {
        return 0;
    }
    vec3 pos_light = frag.pos_world.xyz - pointLights[0].position;
    float curDistance = length(pos_light);
    if (curDistance > farPlane) {
        return 0;
    }
   
    float cos = dot(frag.normal, toLightN);
    float base = 0.0012;
    float fixing = (1 - cos * cos) * (0.033 + base * 14);
    float bias = max(0.001 + base * 7, fixing);
    // bias 带来的悬浮效果 相比平行光 出奇的小？
    
    // float depth = texture(lights.texture_pointLight0, pos_light).r;
    // return float(length(pos_light) > depth * farPlane + bias);
     
    // 透视情况下不太好用偏移一点 xyz 然后采样、卷积，向量太长了。
    // 可以 normalize 一下 pos_lightSpace 再卷
    // 不管怎么卷，三维三次方，kernel 到 3 就已经采样 27 次了，根本不行
    // 换一个方法：就采 20 次，这 20 次是一些固定的 delta 方向！
     
    vec3 posN = normalize(pos_light);
    
    float offsetFixing = curDistance * 0.001 + base;
    // 在阴影中的采样数量
    float total = 0;
    
    for (int i = 0; i < 20; i++) {
        vec3 dir = posN + sampleOffsetDirections[i] * offsetFixing;
        float depth = texture(lights.texture_pointLight0, dir).r;
        total += float(curDistance > depth * farPlane + bias);
    }
    return total / 20;
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

    vec3 amb = light.ambient * tex_color * 0.008;
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
    float pcf = 1 - pointLightShadow(toLightN);

    return (amb + pcf * (diff + spec)) * attenuation;
}
 
/// 完全在阴影中返回 1
float dirLightShadow(mat4 trans, vec3 toLightN) {
   if (!lights.use_texture_dirLight0) {
       return 0;
   }
   vec4 lightSpacePos = trans * frag.pos_world;
   /// get [-1, 1] pos
   vec3 ndcPos = lightSpacePos.xyz / lightSpacePos.w;
   /// [0-1] pos
   ndcPos = ndcPos * 0.5 + 0.5;
   // 视锥短，外面的视为无阴影
   if (ndcPos.z > 1) {
       return 0;
   }
   //float depth = texture(lights.texture_dirLight0, ndcPos.xy).r;
   //// 系数为了表演 Peter Panning 可以调大点
   //float bias = max(0.0001, (1 - abs(dot(frag.normal, toLightN))) * 0.00035);
   //return ndcPos.z > depth + bias;
   
   int kernel_size = 5;
   int half_s = kernel_size / 2;
   vec2 wh_step = 1.0 / textureSize(lights.texture_dirLight0, 0);
   int totalShadow = 0;
   // 感觉效果不太行
   float bias = max(0.0006, (1 - abs(dot(frag.normal, toLightN))) * 0.0015);

   for (int i = 0; i < kernel_size; i++) {
       for (int j = 0; j < kernel_size; j++) {
           float newX = (i - half_s) * wh_step.x + ndcPos.x;
           float newY = (j - half_s) * wh_step.y + ndcPos.y;
           
           float depth = texture(lights.texture_dirLight0, vec2(newX, newY)).r;
           totalShadow += int(ndcPos.z > depth + bias);
       }
   }
   return float(totalShadow) / (kernel_size * kernel_size);
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
    float pcf = 1 - dirLightShadow(light.trans, toLightN);
    return amb + pcf * (diff + spec);
}
