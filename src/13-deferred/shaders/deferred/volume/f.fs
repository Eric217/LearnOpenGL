#version 330 core
layout(location = 0) out vec4 color;
layout(location = 1) out vec4 color1;

struct DeferredTextures {
    sampler2D texture_deferPos0;
    sampler2D texture_deferNormal0;
    sampler2D texture_deferColor0;
};
// MRT 纹理，记得 setTextures
uniform DeferredTextures deferred;

// 常用的变量，记得 bind
layout(std140) uniform Context {
    int context_screen_w;
    int context_screen_h;
    float context_exposure;
    float context_far_plane_ps; // for point shadow

    // vec4 slot1;
};

/// 5 * vec3 的大小，手动对齐到 2 * mat4
struct PointLight {
    vec3 position; // world space coor
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    vec4 attenuation; // k0 k1 k2 radius
    
    vec3 slot0;
    vec3 slot1;
    vec3 slot2;
};

#define NR_POINT_LIGHTS 2

// UBO: 3 * mat4 = NR * 3 * 64
// 光源信息，记得 bind
layout(std140) uniform PointLights {
    PointLight pointLights[NR_POINT_LIGHTS];
};

struct LightMap {
    samplerCube texture_pointLight0;
    bool use_texture_pointLight0;

    samplerCube texture_pointLight1;
    bool use_texture_pointLight1;
};

// 阴影信息，记得 appendTextures
uniform LightMap lights;

// UBO:
// 2 * (4 * 16)
// 记得 bind VP
layout(std140) uniform ViewProjection {
    mat4 view;
    mat4 projection;
};

struct Frag {
    vec3 pos_world;
    vec4 pos_view;
    vec3 normal_view;
    vec3 albedo;
    float specular;
};

vec3 calcPointLight(int idx, PointLight light, Frag frag);

void main() {
    color1 = vec4(0);
    color = vec4(0);
    vec2 tex_coor = vec2(gl_FragCoord.x / context_screen_w, 
                         gl_FragCoord.y / context_screen_h);
    Frag frag;
    frag.pos_world = texture(deferred.texture_deferPos0, tex_coor).xyz;
    frag.normal_view = texture(deferred.texture_deferNormal0, tex_coor).xyz;
        vec4 _tex = texture(deferred.texture_deferColor0, tex_coor);
    frag.albedo = _tex.rgb;
    frag.specular = _tex.a;
   
    vec3 result = vec3(0);  
    if (frag.albedo == result) {
        return;
    } 
    frag.pos_view = view * vec4(frag.pos_world, 1);
    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
        result += calcPointLight(i, pointLights[i], frag);
    } 
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
float pointLightShadow(int idx, vec3 toLightN, Frag frag) {
    if (!lights.use_texture_pointLight0) {
        return 0;
    }
    vec3 pos_light = frag.pos_world - pointLights[idx].position;
    float curDistance = length(pos_light);
    if (curDistance > context_far_plane_ps) {
        return 0;
    }
   
    float cos = dot(frag.normal_view, toLightN);
    float base = 0.0012;
    float fixing = (1 - cos * cos) * (0.033 + base * 14);
    float bias = max(0.001 + base * 7, fixing);
    
    vec3 posN = normalize(pos_light);
    
    float offsetFixing = curDistance * 0.001 + base;
    // 在阴影中的采样数量
    float total = 0;
    
    for (int i = 0; i < 20; i++) {
        vec3 dir = posN + sampleOffsetDirections[i] * offsetFixing;
        float depth = 0;
        if (idx == 0) {
            depth = texture(lights.texture_pointLight0, dir).r;
        } else {
            depth = texture(lights.texture_pointLight1, dir).r;
        }
        total += float(curDistance > depth * context_far_plane_ps + bias);
    }
    return total / 20;
}

/// 其他参数：Payload frag, Material material, MVP
vec3 calcPointLight(int idx, PointLight light, Frag frag) {
    vec3 tex_color = frag.albedo;
    vec4 pos_view = frag.pos_view;

    // eye space coor
    vec4 lightPosView = view * vec4(light.position, 1);
    vec3 toLight = (lightPosView - pos_view).xyz;
    vec3 toLightN = normalize(toLight);
    vec3 toEye = normalize(-pos_view.xyz);

    float cos = max(0, dot(toLightN, frag.normal_view));
    float distance = length(toLight);
    float k0 = light.attenuation.x;
    float k1 = light.attenuation.y;
    float k2 = light.attenuation.z;
    float attenuation = 1.0 / (k0 + k1 * distance + k2 * distance * distance);

    vec3 amb = light.ambient * tex_color * 0.0016;
    vec3 diff = cos * light.diffuse * tex_color;
    // 高光
    vec3 halfV = normalize((toLightN) + toEye);
    
    float specColor = frag.specular;

    // 使用半程而不是反射（Blinn 区别）
    float cos2 = dot(halfV, frag.normal_view);
    vec3 spec;
    if (cos > 0 && cos2 > 0 && dot(toEye, frag.normal_view) > 0) {
        spec = pow(cos2, 15) * specColor * light.specular;
    } else {
        spec = vec3(0);
    }
    float pcf = (1 - pointLightShadow(idx, toLightN, frag)) * attenuation;
    amb *= attenuation;
    color1 += vec4(pcf * spec, 0);
    return pcf * diff + pcf * spec + amb;
}
