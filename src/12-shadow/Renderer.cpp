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
#include <glm/ext.hpp>

using namespace glm;
using namespace config;

#define BINDING_POINT_VP 0
#define BINDING_POINT_DIR_LIGHTS 1

#define DepthFrameBufferSize 1024

/// 预览用的
static Model *depthMap;
#define USE_DEPTH_PREVIEW 0

void Renderer::setup(const Scene &scene) {
    Texture _t;
    
    auto dirLights = scene.getLights<Sun>();
    for (int i = 0; i < dirLights.size(); i++) {
        auto &light = *dirLights[i];
        dirLightBuffer.update(0 + 8 * i, light.direction);
        dirLightBuffer.update(1 + 8 * i, light.ambient);
        dirLightBuffer.update(2 + 8 * i, light.diffuse);
        dirLightBuffer.update(3 + 8 * i, light.specular);
    
        auto b = DepthFramebuffer(DepthFrameBufferSize, DepthFrameBufferSize);
        dirShadowMaps.push_back(b);
        
        light.Light::shader.use();
     
        auto dir = normalize(light.direction);
        auto pos = -30.f * dir;
        auto v = lookAt(pos, pos + dir, vec3(0, 1, 0));
        float frustum = 15.f; // 视锥外面都视为无阴影
        auto pv = ortho(-frustum, frustum, -frustum, frustum, 0.1f, config::cameraFarPlane) * v;
        light.Light::shader.setMat4("vpMatrix", pv);
        dirLightBuffer.update(1 + 2 * i, pv);
        
        auto &models = scene.models;
        Texture t(b.asTexture(), DIR_LIGHT_TEXTURE);
        auto vec = {t};
        _t = t;
        for (auto &model: models) {
            model.get()->appendTextures(vec);
        }
    }
#if USE_DEPTH_PREVIEW
    std::string shaderDir = SHADER_DIR;
    auto previewShader = Shader(shaderDir + "/preview/v.vs", shaderDir + "/preview/f.fs");
    std::string previewModelDir = MODEL_DIR"/../../08-framebuffer/models/quadr/quadr.obj";
    depthMap = new Model(previewModelDir, previewShader, mat4(1), GL_CLAMP_TO_EDGE, false);
    depthMap->setTextures({_t});
#endif
}

void Renderer::render(Scene& scene, const Camera *camera) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vBuffer.update(0, camera->getViewMatrix());
    vBuffer.update(1, config::projectionMatrix(camera->fov));
    
    auto dirLights = scene.getLights<Sun>();
    // 除非没有平面物体产生阴影的需求 否则不要用这个！
    glCullFace(GL_FRONT);
    for (int i = 0; i < dirShadowMaps.size(); i++) {
        dirShadowMaps[i].activate();
        render1(scene, 0, &dirLights[i]->Light::shader);
        dirShadowMaps[i].deactivate();
    }
    glCullFace(GL_BACK);
    glViewport(0, 0, screenPixelW, screenPixelH);
 
#if USE_DEPTH_PREVIEW
    depthMap->draw(true);
#else
    render1(scene, camera);
#endif
}

/// 渲染 models
void Renderer::render1(Scene& scene, const Camera *camera, const Shader *customShader) {
    for (int i = 0; i < scene.models.size(); i++) {
        if (!customShader) {
            scene.modelAt(i).draw(true);
        } else {
            scene.modelAt(i).draw(*customShader);
        }
    }
}

void Renderer::updateScreenSize(int w, int h) {
    glViewport(0, 0, w, h);
}

Renderer::Renderer(const Scene &scene):
    vBuffer(2, GL_STREAM_DRAW),
    dirLightBuffer(2, GL_STATIC_DRAW)
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);
    // 我觉得 cull 效果反而不好，不如 bias？
//    glEnable(GL_CULL_FACE);

    glClearColor(0.01f, 0.01f, 0.01f, 1.f);

    
    // 手动定义槽位意义：绑定好相关的全局 uniform，ubo
    // 0 - view
    // 1 - projection
    // 2 - camera pos
    auto &models = scene.models;
    for (auto &model: models) {
        model.get()->shader.use();
        model.get()->bindUniformBlock("ViewProjection", BINDING_POINT_VP);
        model.get()->bindUniformBlock("DirLights", BINDING_POINT_DIR_LIGHTS);
    }
    vBuffer.bindRange(0, 2 * sizeof(mat4), BINDING_POINT_VP);
    dirLightBuffer.bindRange(0, 2 * 4 * sizeof(vec4), BINDING_POINT_DIR_LIGHTS);
    
    setup(scene);
}
