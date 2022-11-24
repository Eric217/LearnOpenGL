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
    Shader shader;
public:
    virtual void applyToShader(const Shader* shader, const std::string& domain);
};

class AttenuatingLight: public Light {
public:
    float k0;
    float k1;
    float k2;
    
    float r;
public:
    virtual void applyToShader(const Shader* shader, const std::string& domain);
    /// 参数代表光照强度为多少时视为最小可见光，如 5/256，返回值是衰减到这个强度时的半径
    virtual float lightVolumeRadius(float threshold = 4.f/256.f);
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
