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
#include <unordered_map>

// OpenGL 要求最少支持 16 个纹理，每个纹理占 0-15 中的一个位置，位置号就是纹理单元
//
// shader 中，纹理类型是 samplerXD，如 uniform sampler2D texture1;
// 可以通过 glUniform 设置一个纹理的单元号（不指定默认 0），单元号和纹理数据绑定：
// 在 glBindTexture 之前指定当前要绑定到哪个单元：glActiveTexture(unit)
// 如果不指定绑定到哪个单元，默认 bind 到 0
// glActiveTexture 参数 GL_TEXTURE0 是连续的，可以加偏移量得到其他单元

// 生成并配置好一个纹理对象，用于 bind

void Texture::use(int index) const {
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, ID);
}

static std::unordered_map<std::string, Texture> textureCache;

Texture Texture::load(const std::string &path, const std::string &type) {
    auto result = textureCache.find(path);
    if (result != textureCache.end()) {
        auto t = result->second;
        t.type = type;
        return t;
    }
    Texture t(path, type);
    textureCache[path] = t;
    return t;
}

Texture::Texture(const std::string &path, const std::string &t): ID(0), type(t) {
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
   
    // 设置纹理 wrap 方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // 使用 stb 加载图片
    int w, h, channels;
    // 没有反转 Y，需要在 assimp 上反转！
    auto data = stbi_load(path.c_str(), &w, &h, &channels, 0);
    if (!data) {
        std::cout << "Texture data null" << std::endl;
        assert(false);
        return;
    }
    GLenum sourceFormat = GL_RGB;
    if (channels == 4) {
        sourceFormat = GL_RGBA;
    } else if (channels != 3) {
        std::cout << "Channel unacceptable" << std::endl;
        assert(false);
        return;
    }
    // 生成 0 level 的纹理
    glTexImage2D(GL_TEXTURE_2D, 0/*level*/, GL_RGB, w, h, 0, sourceFormat,
                 GL_UNSIGNED_BYTE, data);
    // 生成其他 level 可以重复上个方法，或者直接：
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
    // 由于有 cache 暂不设计释放纹理内存逻辑
    // glDeleteTextures(1, &ID);
}

//Texture::Texture(Texture&& t) {
//    ID = t.ID;
//    type = std::move(t.type);
//}
//
//Texture& Texture::operator=(Texture&& t) {
//    ID = t.ID;
//    type = std::move(t.type);
//    return *this;
//}
//
//Texture& Texture::operator=(Texture& t) {
//    ID = t.ID;
//    type = (t.type);
//    return *this;
//}
