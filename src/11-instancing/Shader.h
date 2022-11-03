//
//  Renderer.hpp
//  01
//
//  Created by Eric on 2022/9/23.
//

#ifndef shader_hpp
#define shader_hpp

#include <string>
#include <glm/glm.hpp>

/// 其实这是 program
class Shader {
public:
    // the program ID
    unsigned int ID;
    
public:
    Shader(): ID(0) {}
    Shader(unsigned int ID): ID(ID) {}
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    Shader(const std::string& vertexPath, const std::string& fragmentPath,
           const std::string& geometryPath);
    
    void use() const;
    
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec3(const std::string &name, const glm::vec3& value) const;
    void setMat4(const std::string &name, const glm::mat4 &matrix) const;
    void setMat3(const std::string &name, const glm::mat4 &matrix) const;
    
    void bindUniformBlock(const std::string &name, int slot) const;    
};


#endif /* Renderer_hpp */
