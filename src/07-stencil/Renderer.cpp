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

// glDepthMask(0);

static mat4 & makeScale(mat4 &mat, float value) {
    mat[0][0] = value;
    mat[1][1] = value;
    mat[2][2] = value;
    return mat;
}

void Renderer::render(Scene& scene) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    shader->use();
    
    shader->setMat4("view", viewM);
    shader->setMat4("projection", projectionM);
    
    // point light pos
    const auto& lightPos = model2M[3];
    shader->setVec3("pointLights[0].position", lightPos);
     
    auto &obj1 = scene.models[0];
    auto &obj1Big = scene.models[1];
    auto &obj2 = scene.models[2];
    auto &obj2Big = scene.models[3];
    
    auto &floor = scene.models[4];
    
    glStencilMask(0xff);
    
    glStencilFunc(GL_ALWAYS, 1, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    floor.draw(*shader);
 
    glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
    shader->use();
    obj1.draw(*shader);
    obj2.draw(*shader);
    
    borderShader->use();
    borderShader->setMat4("view", viewM);
    borderShader->setMat4("projection", projectionM);
    
    glStencilFunc(GL_NOTEQUAL, 1, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    // 画描边时不可以更新深度缓冲
    // glDepthMask(0);
    // 物体可能被遮挡，但我们不希望描边被挡。关闭深度测试

    glDisable(GL_DEPTH_TEST);
    obj1Big.draw(*borderShader);
    obj2Big.draw(*borderShader);
    glEnable(GL_DEPTH_TEST);
    // glDepthMask(1);
    
    glStencilFunc(GL_ALWAYS, 1, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    
    grassShader->use();
    grassShader->setMat4("view", viewM);
    grassShader->setMat4("projection", projectionM);
    grassShader->setVec3("pointLights[0].position", lightPos);
    scene.models[5].draw(*grassShader);
    
    shader->use();
    
    std::vector<Model *> grasses;
    for (int i = 6; i < scene.models.size(); i++) {
        grasses.push_back(&scene.models[i]);
    }
    std::sort(grasses.begin(), grasses.end(), [this](Model *a, Model *b) {
        auto dist = viewM * vec4(a->randomVertex(), 1);
        auto dist2 = viewM * vec4(b->randomVertex(), 1);
        return dot(dist, dist) < dot(dist2, dist2);
    });
    for (auto i = grasses.rbegin(); i != grasses.rend(); i++) {
        (*i)->draw(*shader);
    }
   
    glBindVertexArray(vao);
    // 画灯
    lampShader->use();
    lampShader->setMat4("model", model2M);
    lampShader->setMat4("view", viewM);
    lampShader->setMat4("projection", projectionM);

    glDrawArrays(GL_TRIANGLES, 0, 36);
}

Renderer::Renderer() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);
    
    // 灯不用 enable 这两个
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
    //glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)(6 * sizeof(GLfloat)));
    //glEnableVertexAttribArray(2);

    std::string dir = SHADER_DIR;
    shader = new Shader(dir + "/v.shader", dir + "/f.shader");
    lampShader = new Shader(dir + "/v_light.shader", dir + "/f_light.shader");
    borderShader = new Shader(dir + "/v.shader", dir + "/f_border.shader");
    grassShader = new Shader(dir + "/v.shader", dir + "/f_grass.shader");

    shaders.push_back(shader);
    shaders.push_back(lampShader);
    shaders.push_back(borderShader);
    shaders.push_back(grassShader);
    
 
    {
        auto light2 = PointLight();
        // 位置由实际物体决定，在 render 循环中指定
        light2.ambient = vec3(1);
        light2.diffuse = vec3(1);
        light2.specular = vec3(1);
        light2.k0 = 1;
        light2.k1 = 0.0002;
        light2.k2 = 0.000002;
        shader->use();
        light2.applyToShader(shader, "pointLights[0].");
        grassShader->use();
        light2.applyToShader(grassShader, "pointLights[0].");
    }
    
}

Renderer::~Renderer() {
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &ebo);
 
    for (Shader *shader: shaders) {
        if (shader) {
            delete shader;
        }
    }
    for (Texture *t: textures) {
        if (t) {
            delete t;
        }
    }
} 
