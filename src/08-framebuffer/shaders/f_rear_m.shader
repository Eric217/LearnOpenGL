#version 330 core

out vec4 color;

struct Payload {
    vec2 tex_coor;
};

in Payload frag;

struct Material {
    bool use_texture_diffuse0;
    sampler2D texture_diffuse0;
    // float diffuseValue; 不做那么完整了，没有纹理就用白色
};

struct Screen {
    int width;
    int height;
};

uniform Material material;
uniform Screen screen;

void main() {
    vec3 tex_color = vec3(0, 0, 0);
    if (!material.use_texture_diffuse0) {
        color = vec4(tex_color, 1);
        return;
    }
    vec2 texC = frag.tex_coor;
    texC.y = 1 - texC.y;

    float offsetX = 1.0 / screen.width;
    float offsetY = 1.0 / screen.height;
    
    vec2 offsets[9] = vec2[](
        vec2(-offsetX,  offsetY), // 左上
        vec2( 0.0f,    offsetY), // 正上
        vec2( offsetX,  offsetY), // 右上
        vec2(-offsetX,  0.0f),   // 左
        vec2( 0.0f,    0.0f),   // 中
        vec2( offsetX,  0.0f),   // 右
        vec2(-offsetX, -offsetY), // 左下
        vec2( 0.0f,   -offsetY), // 正下
        vec2( offsetX, -offsetY)  // 右下
    );

    float kernel[9] = float[](
       // 高斯模糊
       // 1.1 / 16, 2.2 / 16, 1.1 / 16,
       // 2.2 / 16, 2.8 / 16, 2.2 / 16,
       // 1.1 / 16, 2.2 / 16, 1.1 / 16
       
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    );
    
    for(int i = 0; i < 9; i++) {
        tex_color += kernel[i] * texture(material.texture_diffuse0, texC + offsets[i]).xyz;
    }
    color = vec4(tex_color, 1);
}
