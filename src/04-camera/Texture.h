//
//  Texture.hpp
//  03
//
//  Created by Eric on 2022/9/27.
//

#ifndef Texture_hpp
#define Texture_hpp

#include <string>

class Texture {
public:
    unsigned int ID;
public:
    Texture(): ID(0) {};
    Texture(const std::string &path);
    ~Texture();
    
    /// 绑定到第几个纹理单元，参数 0 代表 GL_TEXTURE0
    void use(int index);
};

#endif /* Texture_hpp */
