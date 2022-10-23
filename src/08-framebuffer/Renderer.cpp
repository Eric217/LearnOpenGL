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
// 1 activate fb
// 2 render scene to fb
// 3 deactivate fb
// 4 render scene and tex from fb

void Renderer::render(Scene& scene, const Camera *camera) {
    projectionM = config::projectionMatrix(camera->fov);

    viewM = camera->getBackViewMatrix();
   
    fb.activate();
    render1(scene);
    fb.deactivate();
    
    viewM = camera->getViewMatrix();
    glViewport(0, 0, config::screenPixelW, config::screenPixelH);
    render2(scene);
}

/// render scene and tex from fb
void Renderer::render2(Scene& scene) {
    render1(scene);

    auto &mirror = *scene.rearMirror.get();
    mirror.updateTexture(fb.asTexture());
    auto &shader = mirror.prepareDrawing();
    shader.setInt("screen.width", fb.w);
    shader.setInt("screen.height", fb.h);
    
    glDisable(GL_DEPTH_TEST);
    mirror.draw();
    glEnable(GL_DEPTH_TEST);
}

void Renderer::render1(Scene& scene) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    
    // 灯 shader
    auto &lshader = scene.modelAt(5).prepareDrawing();
    lshader.setMat4("view", viewM);
    lshader.setMat4("projection", projectionM);
    scene.modelAt(5).draw();
    
    // 普通物体 shader
    auto &shader = scene.modelAt(0).prepareDrawing();
    shader.setMat4("view", viewM);
    shader.setMat4("projection", projectionM);

    std::vector<const Shader *> litShaders = {&scene.modelAt(6).shader, &shader};
    
    int pointLights = 0;
    for (auto &_light: scene.lights) {
        auto &light = *_light.get();
        auto s = light.name + "[" + std::to_string(pointLights++) + "].";

        for (auto shader: litShaders) {
            shader->use();
            light.applyToShader(shader, s);
        }
    }
     
    auto &obj1 = *scene.models[0].get();
    auto &obj1Big = *scene.models[1].get();
    auto &obj2 = *scene.models[2].get();
    auto &obj2Big = *scene.models[3].get();
    
    auto &floor = *scene.models[4].get();
    
    glStencilFunc(GL_ALWAYS, 1, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glFrontFace(GL_CW);
    shader.use();
    floor.draw(shader);
    
    glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);

    glFrontFace(GL_CCW);
    obj1.draw(shader);
    obj2.draw(shader);
    
    auto &borderShader = scene.modelAt(1).prepareDrawing();
    borderShader.setMat4("view", viewM);
    borderShader.setMat4("projection", projectionM);
    
    glStencilFunc(GL_NOTEQUAL, 1, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glDisable(GL_DEPTH_TEST);
    obj1Big.draw(borderShader);
    obj2Big.draw(borderShader);
    glEnable(GL_DEPTH_TEST);
    
    glStencilFunc(GL_ALWAYS, 1, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    
    glDisable(GL_CULL_FACE);
    auto &grassShader = scene.modelAt(6).prepareDrawing();
    grassShader.setMat4("view", viewM);
    grassShader.setMat4("projection", projectionM);
    scene.modelAt(6).draw();
    
    shader.use();
    
    std::vector<Model *> grasses;
    for (int i = 7; i < scene.models.size(); i++) {
        grasses.push_back(scene.models[i].get());
    }
    std::sort(grasses.begin(), grasses.end(), [this](Model *a, Model *b) {
        auto dist = viewM * vec4(a->randomVertex(), 1);
        auto dist2 = viewM * vec4(b->randomVertex(), 1);
        return dot(dist, dist) < dot(dist2, dist2);
    });
    for (auto i = grasses.rbegin(); i != grasses.rend(); i++) {
        (*i)->draw(shader);
    }
}

static float rearMirrorProportion = 0.2;

void Renderer::updateScreenSize(int w, int h) {
    glViewport(0, 0, w, h);
    fb.updateSize(w * rearMirrorProportion, h * rearMirrorProportion);
}

Renderer::Renderer(): fb(config::screenPixelW * rearMirrorProportion,
                         config::screenPixelH * rearMirrorProportion)
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

Renderer::~Renderer() {
    for (Texture *t: textures) {
        if (t) {
            delete t;
        }
    }
} 
