//
//  Texture.cpp
//  03
//
//  Created by Eric on 2022/9/27.
//

#include "Texture.h"

#include <glad/glad.h>
#include <stb/stb_image.h>
#include <iostream>

// OpenGL 要求最少支持 16 个纹理，每个纹理占 0-15 中的一个位置，位置号就是纹理单元
//
// shader 中，纹理类型是 samplerXD，如 uniform sampler2D texture1;
// 可以通过 glUniform 设置一个纹理的单元号（不指定默认 0），单元号和纹理数据绑定：
// 在 glBindTexture 之前指定当前要绑定到哪个单元：glActiveTexture(unit)
// 如果不指定绑定到哪个单元，默认 bind 到 0
// glActiveTexture 参数 GL_TEXTURE0 是连续的，可以加偏移量得到其他单元

// 生成并配置好一个纹理对象，用于 bind

void Texture::use(int index) {
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, ID);
}

Texture::Texture(const std::string &path): ID(0) {
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
   
    // 设置纹理 wrap 方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // 如果是 border 则设置一个颜色
    float color[] = {0.3, 0.6, 0.5, 1};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
    // 分别设置放缩纹理的插值方法（filter）
    // 对于缩小 需要用到 mipmap，也能指定插值 level 的方法
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // 使用 stb 加载图片
    int w, h, channels;
    stbi_set_flip_vertically_on_load(true);

    auto data = stbi_load(path.c_str(), &w, &h, &channels, 0);
    if (!data) {
        std::cout << "Texture data null" << std::endl;
        return;
    }
    GLenum sourceFormat = GL_RGB;
    if (channels == 4) {
        sourceFormat = GL_RGBA;
    } else if (channels != 3) {
        std::cout << "Channel unacceptable" << std::endl;
        assert(false);
    }
    // 生成 0 level 的纹理
    glTexImage2D(GL_TEXTURE_2D, 0/*level*/, GL_RGB, w, h, 0, sourceFormat,
                 GL_UNSIGNED_BYTE, data);
    // 生成其他 level 可以重复上个方法，或者直接：
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
}

Texture::~Texture() {
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &ID);
}
