//
//  Renderer.hpp
//  01
//
//  Created by Eric on 2022/9/23.
//

#ifndef shader_hpp
#define shader_hpp

#include <string>

class Shader {
public:
    // the program ID
    unsigned int ID;
  
    // constructor reads and builds the shader
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    // use/activate the shader
    void use();
    // utility uniform functions
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
};


#endif /* Renderer_hpp */
