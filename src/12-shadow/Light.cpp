//
//  Light.cpp
//  05-shading
//
//  Created by Eric on 2022/10/9.
//

#include "Light.h"

void Light::applyToShader(const Shader *shader, const std::string &domain) {
    shader->setVec3(domain + "ambient", ambient);
    shader->setVec3(domain + "diffuse", diffuse);
    shader->setVec3(domain + "specular", specular);
}

void AttenuatingLight::applyToShader(const Shader *shader, const std::string &domain) {
    Light::applyToShader(shader, domain);
    shader->setFloat(domain + "k0", k0);
    shader->setFloat(domain + "k1", k1);
    shader->setFloat(domain + "k2", k2);
}

void DirLight::applyToShader(const Shader *shader, const std::string &domain) {
    Light::applyToShader(shader, domain);
    shader->setVec3(domain + "direction", direction);
}

void SpotLight::applyToShader(const Shader *shader, const std::string &domain) {
    AttenuatingLight::applyToShader(shader, domain);
    shader->setVec3(domain + "direction", direction);
    shader->setVec3(domain + "position", position);

    shader->setFloat(domain + "cutOff", cutOff);
    shader->setFloat(domain + "outerCutOff", outerCutOff);
}

void PointLight::applyToShader(const Shader *shader, const std::string &domain) {
    AttenuatingLight::applyToShader(shader, domain);
    shader->setVec3(domain + "position", position);
}
