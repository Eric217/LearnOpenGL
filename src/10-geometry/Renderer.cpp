//
//  Renderer.cpp
//  01
//
//  Created by Eric on 2022/9/23.
//

#include "Renderer.h"
#include "Shader.h"
#include "Light.h"
#include "Config.hpp"

#include <glad/glad.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <algorithm>
#include <vector>
#include <chrono>

using namespace glm;

#define UNIFORM_VIEW 0
#define UNIFORM_PROJECTION 1
#define UNIFORM_CAM 2
#define UNIFORM_EXPLODE 3

void Renderer::render(Scene& scene, const Camera *camera) {
    projectionM = config::projectionMatrix(camera->fov);
    viewM = camera->getViewMatrix();
   
    vBuffer.update(0, viewM);
    pBuffer.update(0, projectionM);
    camBuffer.update(camera->position);
    
    if (GEOMETRY_SHADER > 1) { // explode & normal
        auto value = (duration_cast<std::chrono::milliseconds>
                  (std::chrono::system_clock::now().time_since_epoch())).count();
        float co = 1+std::sin((value % 4000) / 4000.f * 2 * M_PI);
        explodeBuffer.update(0, GEOMETRY_SHADER == 2
                             ? co * 0.29 : (co * 0.18 + 0.02));
    }
    render1(scene, camera);
}
 
void Renderer::render1(Scene& scene, const Camera *camera) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
    {
        for (int i = 0; i < scene.models.size(); i++) {
            scene.modelAt(i).draw(true);
        }
    }
    
    if (scene.skybox) {
        auto &shader = scene.skybox.get()->prepareDrawing();
        shader.setMat3("view", viewM);

        glFrontFace(GL_CW);
        glDepthFunc(GL_LEQUAL);
        scene.skybox.get()->draw();
        glDepthFunc(GL_LESS);
        glFrontFace(GL_CCW);
    }
    
}

void Renderer::updateScreenSize(int w, int h) {
    glViewport(0, 0, w, h);
}

Renderer::Renderer(const Scene &scene) {
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    glLineWidth(3);
    
    // 手动定义槽位意义：绑定好相关的全局 uniform，ubo
    // 0 - view
    // 1 - projection
    // 2 - camera pos
    for (auto &model: scene.models) {
        model.get()->shader.use();
        model.get()->bindUniformBlock("View", UNIFORM_VIEW);
        model.get()->bindUniformBlock("Projection", UNIFORM_PROJECTION);
        model.get()->bindUniformBlock("Explosion", UNIFORM_EXPLODE);
    }
    // 其他特殊处理
    if (scene.skybox) {
        scene.skybox.get()->shader.use();
        // box 只要一个 projection 绑定
        scene.skybox.get()->bindUniformBlock("Projection", UNIFORM_PROJECTION);
    }
    scene.models[0].get()->shader.use();
    scene.models[0].get()->bindUniformBlock("Uniforms", UNIFORM_CAM);
    
    vBuffer.bindRange(0, sizeof(mat4), UNIFORM_VIEW);
    pBuffer.bindRange(0, sizeof(mat4), UNIFORM_PROJECTION);
    camBuffer.bindRange(0, sizeof(vec3), UNIFORM_CAM);
    explodeBuffer.bindRange(0, sizeof(float), UNIFORM_EXPLODE);
}

Renderer::~Renderer() {
} 
