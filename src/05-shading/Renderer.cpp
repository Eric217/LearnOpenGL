//
//  Renderer.cpp
//  01
//
//  Created by Eric on 2022/9/23.
//

#include "Renderer.h"
#include "Shader.h"
#include "Light.h"

#include <glad/glad.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <algorithm>

float vertices[] = {
    // positions          // normals           // texture coords
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
    0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
    
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
};
 
using namespace glm;

void Renderer::render() {
    shader->use();
    shader->setMat4("model", modelM);
    shader->setMat4("view", viewM);
    shader->setMat4("projection", projectionM);
    
    // point light pos
    const auto& lightPos = model2M[3];
    shader->setVec3("pointLights[0].position", lightPos);
    // spot light pos
    shader->setVec3("spotLights[0].position", camera->position);
    shader->setVec3("spotLights[0].direction", camera->front);
    
    shader->setMat4("normalTransform", inverse(transpose(viewM * modelM)));
    
    glBindVertexArray(vao);
    // 前面需要开启深度测试！
    glDrawArrays(GL_TRIANGLES, 0, 36);
   
    // 画灯
    lampShader->use();
    lampShader->setMat4("model", model2M);
    lampShader->setMat4("view", viewM);
    lampShader->setMat4("projection", projectionM);
    
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

Renderer::Renderer() {
    glEnable(GL_DEPTH_TEST);
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    std::string dir = SHADER_DIR;
    shader = new Shader(dir + "/v.shader", dir + "/f.shader");
    lampShader = new Shader(dir + "/v.shader", dir + "/f2.shader");
   
    dir = TEXTURE_DIR;
    textures.push_back(new Texture(dir + "/container2.png"));
    textures.push_back(new Texture(dir + "/container2_specular.png"));
    textures.push_back(new Texture(dir + "/matrix.jpg"));
    
    shader->use();
    // 给 shader 的纹理对象设置单元号，然后把纹理数据绑定到单元号
    shader->setInt("material.diffuse", 0);
    textures[0]->use(0);
    shader->setInt("material.specular", 1);
    textures[1]->use(1);
    shader->setInt("material.emission", 2);
    textures[2]->use(2);
    shader->setFloat("material.shininess", 150);
    
    {
        auto light = DirLight();
        light.direction = vec3(5, -4, -8);
        light.ambient = vec3(1);
        light.diffuse = vec3(1);
        light.specular = vec3(1);
        light.applyToShader(shader, "dirLights[0].");
    }
    {
        auto light2 = PointLight();
        // 位置由实际物体决定，在 render 循环中指定
        light2.ambient = vec3(1);
        light2.diffuse = vec3(1);
        light2.specular = vec3(1);
        light2.k0 = 1;
        light2.k1 = 0.05;
        light2.k2 = 0.005;
        light2.applyToShader(shader, "pointLights[0].");
    }
    {
        auto light = SpotLight();
        light.ambient = vec3(1);
        light.diffuse = vec3(1);
        light.specular = vec3(1);
        light.k0 = 1;
        light.k1 = 0.015;
        light.k2 = 0.003;
        light.cutOff = 0.992;
        light.outerCutOff = light.cutOff - 0.005;
        light.applyToShader(shader, "spotLights[0].");
    }
    
}

Renderer::~Renderer() {
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &ebo);
 
    if (shader) {
        delete shader;
    }
    if (lampShader) {
        delete lampShader;
    }
    for (Texture *t: textures) {
        if (t) {
            delete t;
        }
    }
} 
