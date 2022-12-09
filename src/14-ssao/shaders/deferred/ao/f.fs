#version 330 core
// out gl_FragDepth

struct DeferredTextures {
    sampler2D texture_deferPos0;    
    sampler2D texture_deferNormal0; 
    sampler2D texture_aoNoise0;
};
// MRT 纹理
uniform DeferredTextures deferred;

// 纹理坐标，或用 gl_FragCoord
in vec2 tex_coor;
   
// 局部空间采样点
// 大小：4 + 4 + ... + 64 个 vec4 = 16 + 1 个 mat4
layout(std140) uniform AO_Config {
    vec4 samples[64];  
    int kernel_size; // 使用 64 个采样中的几个[8-64,64]
    float radius; // 采样半径[0.01-1,1]
    float intensity; // 遮蔽因子的指数参数，pow(occlusion, 1/intensity)[0.1-100,1]
    float gather; // 让分布贴近圆心，sample = pow(length(sample), gather)[0.01-10,2]
    float attenuation; // range check 用的，当遮挡很远时 减弱遮挡的影响[0-1, 0.22]
};

// 相机配置 
layout(std140) uniform ViewProjection {
    mat4 view;
    mat4 projection;
};

struct Frag {
    vec3 pos_world;
    vec3 pos_view;
    float z_view;
    vec3 normal_view;
    vec3 noise; 
};
 
void main() { 
    // prepare frag 
    Frag frag;
    vec4 _pos = texture(deferred.texture_deferPos0, tex_coor);
    frag.pos_world = _pos.xyz;
    frag.z_view = _pos.w;
    frag.pos_view = (view * vec4(frag.pos_world, 1)).xyz;
    frag.normal_view = normalize(texture(deferred.texture_deferNormal0, tex_coor).xyz);
    vec2 _size = textureSize(deferred.texture_deferPos0, 0);
    frag.noise = texture(deferred.texture_aoNoise0, tex_coor * _size / 4).xyz;
    // 需要一种办法检测 quad 中一个点是否有对应的物体，我简单处理为 normal == 0
    if (frag.normal_view == vec3(0)) {
        gl_FragDepth = 1;
        return;
    } 

    // get tbn
    vec3 T = normalize(frag.noise - dot(frag.normal_view, frag.noise) * frag.normal_view); 
    vec3 B = cross(frag.normal_view, T);
    mat3 tbn = mat3(T, B, frag.normal_view);
    
    float occ = 0;
    for (int i = 0; i < kernel_size; i++) {
        vec3 sample = samples[i].xyz; // 长度 0-1
        sample *= mix(0.08, 1.0, pow(length(sample), gather)); // 0-1
        // 半径调整一下
        // 注意 sample 的 z 是负数！！！
        sample = frag.pos_view + radius * (tbn * sample);
        _pos = projection * vec4(sample, 1);
      
        _pos.xyz /= _pos.w; // NDC 
        // 获取采样点的深度，进行比较
        _pos.xy = (_pos.xy + 1) * 0.5; // x y: 0-1
        // 越靠后，z_v 越小，如果 z_v 很大说明遮挡了
        float z_v = texture(deferred.texture_deferPos0, _pos.xy).w;
        // 距离大的贡献小
        float contribution = smoothstep(0.0, 1.0, 
            attenuation * radius / max(0.0001f, abs(frag.z_view - z_v)));
        occ += sample.z < z_v ? contribution : 0;
    }
    // 遮蔽增强，如 0.5, 0.5^0.5 -> 0.7, 0.5^2 -> 0.25
    occ = pow(occ / kernel_size, 1 / intensity);

    gl_FragDepth = occ;
} 
 
