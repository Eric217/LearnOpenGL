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

/*
Stencil Testing
 frag shader -> depth test -> stencil test -> blending
 模板缓冲中每个 stencil value 一般是 8bit，256个取值范围。
 描边思路：
 1 把地板画上
 2 把物体画上，更新 stencil buffer 1
 3 放大物体，关掉深度、只画 stencil = 0 的部分，用单独的 frag shader
 相关：后面再画新物体仍会挡住描边

Blending
 两个 test 通过后，用该 frag color 与 buffer color 进行 blend，使用指定的混合函数。
 blending 思路：
 先绘制所有不参加混合的 即不透明物体；对透明物体对 eye 距离排序再混合，
 这一点比较难做，物体可能部分重合、可能需要每像素考虑等等，
 有次序无关透明度(Order Independent Transparency, OIT)
  
Face culling
 实现算法：
 如果用面的法线与视线的夹角（大于 90 度剔除）太复杂；
 用 shoelace algorithm 思路：vertex shader 之后直接用该算法对三个点计算面积，如果面积为负数则是在 back 剔除。算法：https://www.youtube.com/watch?v=wpNef1Nu4pA ，算法固定逆时针处理三个点，得出来的是正数则是正面，负数是反面。

 */

void Renderer::render(Scene& scene) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glBindVertexArray(vao);
    // 画灯
    lampShader->use();
    lampShader->setMat4("model", model2M);
    lampShader->setMat4("view", viewM);
    lampShader->setMat4("projection", projectionM);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
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
    glFrontFace(GL_CW);
    floor.draw(*shader);
 
    glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
    shader->use();
    glFrontFace(GL_CCW);
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
    
    glDisable(GL_CULL_FACE);
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
