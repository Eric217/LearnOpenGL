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
// shader 中，纹理类型是 samplerXX，如 uniform sampler2D texture1;
// 可以通过 glUniform 设置一个纹理的单元号（不指定默认 0），单元号和纹理数据绑定：
// 在 glBindTexture 之前指定当前要绑定到哪个单元：glActiveTexture(unit)
// 如果不指定绑定到哪个单元，默认 bind 到 0
// glActiveTexture 参数 GL_TEXTURE0 是连续的，可以加偏移量得到其他单元

// 生成并配置好一个纹理对象，用于 bind

// cubemap 也需要激活插槽，不能和别的纹理重复用相同槽！！！

void Texture::use(int index, GLenum wrap) const {
    glActiveTexture(GL_TEXTURE0 + index);

    if (type == CUBEMAP_TEXTURE) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    } else if (type == CUBEMAP2D_TEXTURE) {
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    } else {
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    }
}

static std::unordered_map<std::string, Texture> textureCache;

Texture Texture::load(const std::string &path, const std::string &type, bool useLevels) {
    auto k = path + "::" + std::to_string(useLevels);
    auto result = textureCache.find(k);
    if (result != textureCache.end()) {
        auto t = result->second;
        t.type = type;
        return t;
    }
    Texture t;
    if (type == CUBEMAP_TEXTURE) {
        t = CubeTexture(path);
    } else {
        t = Texture(path, type, useLevels, type == DIFFUSE_TEXTURE);
    }
    textureCache[k] = t;
    return t;
}

CubeTexture::CubeTexture(const std::string &path) {
    shouldUse = true;
    type = CUBEMAP_TEXTURE;
   
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    
    int w, h, channels;
    stbi_set_flip_vertically_on_load(false);

    for (int i = 0; i < 6; i++) {
        auto data = stbi_load((path + std::to_string(i) + ".jpg").c_str(), &w, &h, &channels, 0);
        if (!data) {
            assert(false);
            return;
        }
        GLenum sourceFormat = GL_RGB;
        if (channels == 4) {
            sourceFormat = GL_RGBA;
        } else if (channels != 3) {
            assert(false);
            return;
        }
        // 生成 0 level 的纹理
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, sourceFormat, w, h,
                     0/*border*/, sourceFormat, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    stbi_set_flip_vertically_on_load(true);
}

Texture::Texture(const std::string &path, const std::string &t, bool useLevels, bool useSRGB): ID(0), type(t), shouldUse(true) {
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
   
    // 设置纹理 wrap 方式、放大放小插值方式
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
 
    GLenum minF = useLevels ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minF);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // 使用 stb 加载图片
    int w, h, channels;
    // 反转 Y 在这里做，assimp 不要重复做！
    stbi_set_flip_vertically_on_load(true);
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
    GLenum dstFormat = sourceFormat;
    if (useSRGB) {
        dstFormat = sourceFormat == GL_RGB ? GL_SRGB : GL_SRGB_ALPHA;
    }
    // 生成 0 level 的纹理
    glTexImage2D(GL_TEXTURE_2D, 0/*level*/, dstFormat, w, h, 0, sourceFormat,
                 GL_UNSIGNED_BYTE, data);
    if (useLevels) {
        // 生成其他 level 可以重复上个方法，或者直接：
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    stbi_image_free(data);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}
