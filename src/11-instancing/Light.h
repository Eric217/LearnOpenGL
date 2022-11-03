//
//  Light.hpp
//  05-shading
//
//  Created by Eric on 2022/10/9.
//

#ifndef Light_hpp
#define Light_hpp

#include "Shader.h"
#include <glm/glm.hpp>
#include <string.h>

using namespace glm;

class Light {
public:
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    std::string name;
    
public:
    virtual void applyToShader(const Shader* shader, const std::string& domain);
};

class AttenuatingLight: public Light {
public:
    float k0;
    float k1;
    float k2;
public:
    virtual void applyToShader(const Shader* shader, const std::string& domain);
};

class DirLight: public Light {
public:
    vec3 direction; // world space coor
    
public:
    void applyToShader(const Shader* shader, const std::string& domain);
};

class PointLight: public AttenuatingLight {
public:
    vec3 position; // world space coor

public:
    PointLight() {}
    PointLight(const vec3 &pos): position(pos) {}
    void applyToShader(const Shader* shader, const std::string& domain);
};

class SpotLight: public AttenuatingLight {
public:
    vec3 position; // world space coor
    vec3 direction; // world space coor

    float cutOff;
    float outerCutOff;

public:
    void applyToShader(const Shader* shader, const std::string& domain);
};

#endif /* Light_hpp */
