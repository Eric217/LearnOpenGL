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
#include <map>

class Texture;

enum TextureType {
    diffuse, specular, cubemap2d, cubemap, dirlight, pointlight,
    bloom
};
extern const std::map<TextureType, std::string> TextureTypeName;

class Texture {
public:
    unsigned int ID;
    TextureType type;
    bool shouldUse;
public:
    static Texture load(const std::string &path, TextureType type, bool useLevels = true);
    Texture(): ID(0), shouldUse(true) {};
    Texture(unsigned int ID, TextureType type): ID(ID), type(type), shouldUse(true) {};
    
    /// 绑定到第几个纹理单元，参数 0 代表 GL_TEXTURE0
    void use(int index, GLenum wrap = GL_REPEAT) const;
    
    const std::string& typeName() const {
        return TextureTypeName.find(type)->second;
    }
protected:
    Texture(const std::string &path, TextureType type, bool useLevels, bool useSRGB);
};

/// 偷懒继承用的，实际两个是 siblings
class CubeTexture: public Texture {
public:
    CubeTexture() {}
    CubeTexture(const std::string &path);
};

#endif /* Texture_hpp */
