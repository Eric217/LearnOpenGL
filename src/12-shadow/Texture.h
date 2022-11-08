//
//  Texture.hpp
//  03
//
//  Created by Eric on 2022/9/27.
//

#ifndef Texture_hpp
#define Texture_hpp

#include <string>
#include <glad/glad.h>

// shader 中纹理顺序必须按下面顺序摆放，激活单元绑定时按这个顺序绑定
#define DIFFUSE_TEXTURE "texture_diffuse"
#define SPECULAR_TEXTURE "texture_specular"
#define CUBEMAP2D_TEXTURE "texture_cubemap2d"
#define CUBEMAP_TEXTURE "texture_cubemap"
#define DIR_LIGHT_TEXTURE "texture_dirLight"

class Texture {
public:
    unsigned int ID;
    std::string type;
    bool shouldUse;
public:
    static Texture load(const std::string &path, const std::string &type, bool useLevels = true);
    Texture(): ID(0), shouldUse(true) {};
    Texture(unsigned int ID, const std::string &type): ID(ID), type(type), shouldUse(true) {};
    
    /// 绑定到第几个纹理单元，参数 0 代表 GL_TEXTURE0
    void use(int index, GLenum wrap = GL_REPEAT) const;

protected:
    Texture(const std::string &path, const std::string &type, bool useLevels, bool useSRGB);
};

/// 偷懒继承用的，实际两个是 siblings
class CubeTexture: public Texture {
public:
    CubeTexture() {}
    CubeTexture(const std::string &path);
};

#endif /* Texture_hpp */
