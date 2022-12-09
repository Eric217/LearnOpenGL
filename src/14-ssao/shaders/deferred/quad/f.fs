#version 330 core
layout(location = 0) out vec4 color; 
layout(location = 1) out vec4 color1;

struct DeferredTextures {
    sampler2D texture_deferPos0;
    sampler2D texture_deferNormal0;
    sampler2D texture_deferColor0;
    sampler2D texture_aoMap0;
    bool use_texture_aoMap0;
};
// MRT 纹理
uniform DeferredTextures deferred;
// 纹理坐标，或用 gl_FragCoord
in vec2 tex_coor;
 
struct DirLight {
    vec3 direction; // world space coor
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    mat4 trans;
};

#define NR_DIR_LIGHTS 1
 
// UBO: 4 * vec3 + mat4 = NR * 8 * 16
// 光信息
layout(std140) uniform DirLights {
    DirLight dirLights[NR_DIR_LIGHTS];
};

struct LightMap {
    sampler2D texture_dirLight0;
    bool use_texture_dirLight0;
};

// 阴影信息
uniform LightMap lights;

// 相机配置
// UBO:
// 2 * (4 * 16)
layout(std140) uniform ViewProjection {
    mat4 view;
    mat4 projection;
};

struct Frag {
    vec3 pos_world;
    vec3 normal_view;
    vec3 albedo;
    float specular;
    float ambOcclu;
};

vec3 calcDirectionalLight(DirLight light, Frag frag);

void main() { 
    color = vec4(0);
    color1 = vec4(0);
    Frag frag;
    frag.pos_world = texture(deferred.texture_deferPos0, tex_coor).xyz;
    frag.normal_view = texture(deferred.texture_deferNormal0, tex_coor).xyz;
        vec4 _tex = texture(deferred.texture_deferColor0, tex_coor);
    frag.albedo = _tex.rgb;
    frag.specular = _tex.a;
    frag.ambOcclu = deferred.use_texture_aoMap0 ? texture(deferred.texture_aoMap0, tex_coor).r : 0;
   
    vec3 result = vec3(0);  
    if (frag.albedo == result) {
        return;
    } 
    for (int i = 0; i < NR_DIR_LIGHTS; i++) {
        result += calcDirectionalLight(dirLights[i], frag);
    } 
    color = vec4(result, 1); 
} 

/// 完全在阴影中返回 1
float dirLightShadow(mat4 trans, vec3 toLightN, Frag frag) {
   if (!lights.use_texture_dirLight0) {
       return 0;
   }
   vec4 lightSpacePos = trans * vec4(frag.pos_world, 1);
   /// get [-1, 1] pos
   vec3 ndcPos = lightSpacePos.xyz / lightSpacePos.w;
   /// [0-1] pos
   ndcPos = ndcPos * 0.5 + 0.5;
   // 视锥短，外面的视为无阴影
   if (ndcPos.z > 1) {
       return 0;
   }  
   int kernel_size = 5;
   int half_s = kernel_size / 2;
   vec2 wh_step = 1.0 / textureSize(lights.texture_dirLight0, 0);
   int totalShadow = 0;
   // 感觉效果不太行
   float bias = max(0.0006, (1 - abs(dot(frag.normal_view, toLightN))) * 0.005);

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
vec3 calcDirectionalLight(DirLight light, Frag frag) {
    vec3 tex_color = frag.albedo;
    vec3 toLightN = normalize(-vec3(view * vec4(light.direction, 0)));
    vec3 toEye = normalize(-(view * vec4(frag.pos_world, 1)).xyz);

    float cos = max(0, dot(toLightN, frag.normal_view));
    // 0.003
    vec3 amb = light.ambient * tex_color * 0.1 * (1 - frag.ambOcclu);
    vec3 diffu = cos * light.diffuse * tex_color;
    // 高光
    vec3 halfV = normalize(toLightN + toEye);
         
    // 使用半程而不是反射（Blinn 区别）
    float cos2 = dot(halfV, frag.normal_view);
    vec3 spec;
    if (cos > 0 && cos2 > 0 && dot(toEye, frag.normal_view) > 0) {
        spec = pow(cos2, 15) * frag.specular * light.specular;
    } else {
        spec = vec3(0);
    }
    float pcf = 1 - dirLightShadow(light.trans, toLightN, frag);
    color1 += vec4(pcf * spec, 0);
    return amb + pcf * (diffu + spec);
}
