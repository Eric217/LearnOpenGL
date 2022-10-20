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

class Texture {
public:
    unsigned int ID;
    std::string type;
public:
    static Texture load(const std::string &path, const std::string &type);
    Texture(): ID(0) {};
//    Texture(const Texture& t): ID(t.ID), type(t.type) {};
//    Texture(Texture&& t);
//    Texture& operator=(Texture& t);
//    Texture& operator=(Texture&& t);
    ~Texture();

    /// 绑定到第几个纹理单元，参数 0 代表 GL_TEXTURE0
    void use(int index, GLenum wrap = GL_REPEAT) const;

protected:
    Texture(const std::string &path, const std::string &type);
};

#endif /* Texture_hpp */
