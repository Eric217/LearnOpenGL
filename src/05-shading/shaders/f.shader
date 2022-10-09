#version 330 core

// 光的颜色 * 物体反射的颜色 = 表现的颜色

// 着色的三个部分 ambient diffuse specular
//
// 一般物体 ambient 颜色就是 diffuse 的颜色，只是光比较弱（但实际代码可能不这样写）
// ambient 如果想 也可以单独贴图/固定值，但一般直接使用 diffuse 贴图的颜色
//
// diffuse 一般就是用一个漫反射贴图
//
// specular 如果物体表面是均一的，可以用固定高光度；如果三角形上有些部分是 diffuse，
// 有些是 specular，则使用镜面光贴图，一般使用黑白的，因为高光大多由光源决定颜色。

struct Material {
    // 这里让 ambient 等于 diffuse * 0.05，简化处理
    sampler2D diffuse; // 简化处理：只支持贴图
    sampler2D specular; // 同上。可能 shader 和对象应该绑定？就不用处理不同材质了
    
    sampler2D emission; // 自发光
    // 反光度
    float shininess;
};

uniform Material material;


// 光源类型：1 平行光，只需要光的方向 2 点光源，随距离衰减 3 聚光，有角度范围 + 衰减
// 衰减遵循 1 / (k0 + k1 * distance + k2 * distance * distance)
// 聚光有内外环用于模糊边界

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
#define NR_DIR_LIGHTS 1
#define NR_SPOT_LIGHTS 1

uniform DirLight dirLights[NR_DIR_LIGHTS];
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[NR_SPOT_LIGHTS];


struct Payload {
    vec2 tex_coor;
    vec3 pos; // view space coor
    vec3 normal; // view space coor
};

in Payload frag;

// 其他
uniform mat4 view; // vertex shader 的 view matrix

out vec4 color;


vec3 calcPointLight(PointLight light);
vec3 calcSpotLight(SpotLight light);
vec3 calcDirectionalLight(DirLight light);

// 如果在顶点着色器中实现着色，然后对颜色插值就是 Gouraud shading
// 如果在顶点着色器输出的颜色改用 flat 而非默认的 smooth，就是 flat shading
// 现在是插值法线、在片元着色器着色，就是 Phong shading
void main() {
    vec3 result = vec3(0);
    for (int i = 0; i < NR_DIR_LIGHTS; i++) {
        result += calcDirectionalLight(dirLights[i]);
    }
    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
        result += calcPointLight(pointLights[i]);
    }
    for (int i = 0; i < NR_SPOT_LIGHTS; i++) {
        result += calcSpotLight(spotLights[i]);
    }
    if (dot(frag.normal, normalize(-frag.pos)) < 0.3) {
        result += texture(material.emission, frag.tex_coor).xyz;
    }
    color = vec4(result, 1);
}

/// 其他参数：Payload frag, Material material, MVP
vec3 calcPointLight(PointLight light) {
    vec3 tex_color = texture(material.diffuse, frag.tex_coor).xyz;
    // eye space coor
    vec3 toLight = (view * vec4(light.position, 1) - vec4(frag.pos, 1)).xyz;
    vec3 toLightN = normalize(toLight); 
    vec3 toEye = normalize(-frag.pos);

    float cos = max(0, dot(toLightN, frag.normal));
    float distance = length(toLight);
    float attenuation = 1.0 
        / (light.k0 + light.k1 * distance + light.k2 * distance * distance);

    vec3 amb = light.ambient * tex_color * 0.025;
    vec3 diff = cos * light.diffuse * tex_color;
    // 高光
    vec3 halfV = normalize((toLightN) + toEye);
    tex_color = texture(material.specular, frag.tex_coor).xyz;
    // 使用半程而不是反射（Blinn 区别）
    float cos2 = dot(halfV, frag.normal);
    vec3 spec;
    if (cos > 0 && cos2 > 0 && dot(toEye, frag.normal) > 0) {
        spec = pow(cos2, material.shininess) * light.specular * tex_color;
    } else {
        spec = vec3(0);
    }
    return (amb + diff + spec) * attenuation; 
}

/// 其他参数：Payload frag, Material material, MVP
vec3 calcSpotLight(SpotLight light) {
    vec3 tex_color = texture(material.diffuse, frag.tex_coor).xyz;
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
    
    vec3 amb = light.ambient * tex_color * 0.005;

    cos = max(0, dot(frag.normal, toLightN));
    vec3 diff = cos * light.diffuse * tex_color * intensity;
    // 高光
    vec3 halfV = normalize((toLightN) + toEye);
    // 使用半程而不是反射（Blinn 区别）
    float cos2 = dot(halfV, frag.normal);
    vec3 spec;
    if (intensity > 0 && cos > 0 && cos2 > 0 && dot(toEye, frag.normal) > 0) {
        tex_color = texture(material.specular, frag.tex_coor).xyz;
        spec = pow(cos2, material.shininess) * light.specular * tex_color * intensity;
    } else {
        spec = vec3(0);
    }
    return (amb + diff + spec) * attenuation; 
}

/// 其他参数：Payload frag, Material material, MVP
vec3 calcDirectionalLight(DirLight light) {
    vec3 tex_color = texture(material.diffuse, frag.tex_coor).xyz;
    vec3 toLightN = normalize(-vec3(view * vec4(light.direction, 0)));
    vec3 toEye = normalize(-frag.pos);

    float cos = max(0, dot(toLightN, frag.normal));

    vec3 amb = light.ambient * tex_color * 0.05;
    vec3 diff = cos * light.diffuse * tex_color;
    // 高光
    vec3 halfV = normalize((toLightN) + toEye);
    tex_color = texture(material.specular, frag.tex_coor).xyz;
    // 使用半程而不是反射（Blinn 区别）
    float cos2 = dot(halfV, frag.normal);
    vec3 spec;
    if (cos > 0 && cos2 > 0 && dot(toEye, frag.normal) > 0) {
        spec = pow(cos2, material.shininess) * light.specular * tex_color;
    } else {
        spec = vec3(0);
    }
    return amb + diff + spec;
}
