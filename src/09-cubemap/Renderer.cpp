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

using namespace glm;
 
void Renderer::render(Scene& scene, const Camera *camera) {
    projectionM = config::projectionMatrix(camera->fov);
    viewM = camera->getViewMatrix();

    render1(scene);
}
 
void Renderer::render1(Scene& scene) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
    {
        auto &model = scene.modelAt(0);
        auto &shader = model.prepareDrawing();
        applyViewProjectionMatrix(shader);
        model.draw();
    }
    
    if (scene.skybox) {
        auto &shader = scene.skybox.get()->prepareDrawing();
        shader.setMat3("view", viewM);
        shader.setMat4("projection", projectionM);
        glFrontFace(GL_CW);
        glDepthFunc(GL_LEQUAL);
        scene.skybox.get()->draw();
        glDepthFunc(GL_LESS);
        glFrontFace(GL_CCW);
    }
    
}

void Renderer::applyViewProjectionMatrix(const Shader &shader) const {
    shader.setMat4("view", viewM);
    shader.setMat4("projection", projectionM);
}

void Renderer::updateScreenSize(int w, int h) {
    glViewport(0, 0, w, h);
}

Renderer::Renderer() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

Renderer::~Renderer() {
    for (Texture *t: textures) {
        if (t) {
            delete t;
        }
    }
} 
