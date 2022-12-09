//
//  Light.cpp
//  05-shading
//
//  Created by Eric on 2022/10/9.
//

#include "Light.h"
#include <math.h>

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

float AttenuatingLight::lightVolumeRadius(float threshold) {
    // (-b + (b2-4ac)^0.5) / 2a
    // a = k2, b = k1, c = k0 - 1/t
    float intensity = diffuse.r;
    return (-k1 + sqrt(k1 * k1 - 4 * k2 * (k0 - intensity/threshold))) / (2 * k2);
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
