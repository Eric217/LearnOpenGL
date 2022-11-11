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
#define BINDING_POINT_POINT_LIGHTS 2
#define BINDING_POINT_CUBE_MAT 3

constexpr int dirLightCount = 1;
constexpr int dirLightMat4Count = 2;

constexpr int pointLightCount = 1;
constexpr int pointLightMat4Count = 2;

constexpr float pointShadowFarPlane = 20;

#define DepthFrameBufferSize 1024

/// 预览用的
static Model *depthMap;
#define USE_DEPTH_PREVIEW 0
#define USE_CUBE_PREVIEW 0
#define USE_FRONT_CULLING 0

void Renderer::setup(const Scene &scene) {
    Texture t1, t2;
    
    auto dirLights = scene.getLights<Sun>();
    for (int i = 0; i < dirLights.size(); i++) {
        auto &light = *dirLights[i];
        dirLightBuffer.update(0 + 4 * dirLightMat4Count * i, light.direction);
        dirLightBuffer.update(1 + 4 * dirLightMat4Count * i, light.ambient);
        dirLightBuffer.update(2 + 4 * dirLightMat4Count * i, light.diffuse);
        dirLightBuffer.update(3 + 4 * dirLightMat4Count * i, light.specular);
    
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
        t1 = t;
        for (auto &model: models) {
            model.get()->appendTextures(vec);
        }
        light.Model::shader.use();
        light.Model::shader.setVec3("lightColor", light.ambient);
    }
    auto pointLights = scene.getLights<Bulb>();
    for (int i = 0; i < pointLights.size(); i++) {
        auto &light = *dynamic_cast<PointLight*>(pointLights[i]);
        auto &model = *dynamic_cast<Model*>(pointLights[i]);
        
        light.shader.use();
        light.shader.setFloat("farPlane", pointShadowFarPlane);
        light.shader.setVec4("cameraPos", vec4(light.position, 1));
        light.shader.bindUniformBlock("Transforms", BINDING_POINT_CUBE_MAT);
        cubeMatrices.bindRange(0, 6 * sizeof(mat4), BINDING_POINT_CUBE_MAT);

        model.shader.use();
        model.shader.setVec3("lightColor", light.ambient);
        
        auto b = CubeDepthFramebuffer(DepthFrameBufferSize, DepthFrameBufferSize);
        pointShadowMaps.push_back(b);
        
        Texture t(b.asTexture(), POINT_LIGHT_TEXTURE);
        t2 = t;
        auto vec = {t};
        auto &models = scene.models;
        for (auto &model: models) {
            model.get()->appendTextures(vec);
        }
        pointLightBuffer.update(0 + 4 * pointLightMat4Count * i, light.position);
        pointLightBuffer.update(1 + 4 * pointLightMat4Count * i, light.ambient);
        pointLightBuffer.update(2 + 4 * pointLightMat4Count * i, light.diffuse);
        pointLightBuffer.update(3 + 4 * pointLightMat4Count * i, light.specular);
        pointLightBuffer.update(4 + 4 * pointLightMat4Count * i,
                                vec3(light.k0, light.k1, light.k2));
 
        mat4 proj = config::perspective(90, 1, 0.5, pointShadowFarPlane);
        // 关于这里 lookat 的 up vec，因为是给 cubemap 用的，
        // cubemap 会假设纹理是从外面看的样子，他内部会有 1-x 的操作，
        // 所以我们绘制一个面的时候要反着画!
        cubeMatrices.update(0, proj * lookAt(light.position, light.position + vec3(1,0,0), vec3(0,-1,0)));
        cubeMatrices.update(1, proj * lookAt(light.position, light.position + vec3(-1,0,0), vec3(0,-1,0)));
        cubeMatrices.update(2, proj * lookAt(light.position, light.position + vec3(0,1,0), vec3(0,0,1)));
        cubeMatrices.update(3, proj * lookAt(light.position, light.position + vec3(0,-1,0), vec3(0,0,-1)));
        cubeMatrices.update(4, proj * lookAt(light.position, light.position + vec3(0,0,1), vec3(0,-1,0)));
        cubeMatrices.update(5, proj * lookAt(light.position, light.position + vec3(0,0,-1), vec3(0,-1,0)));
    }
    
#if USE_DEPTH_PREVIEW
    auto previewShader = Shader(SHADER_DIR"/preview/dir.vs", SHADER_DIR"/preview/dir.fs");
    std::string previewModelDir = MODEL_DIR"/../../08-framebuffer/models/quadr/quadr.obj";
    depthMap = new Model(previewModelDir, previewShader, mat4(1), GL_CLAMP_TO_EDGE, false);
    t2.shouldUse = false;
    depthMap->setTextures({t1});
#endif
#if USE_CUBE_PREVIEW
    auto previewShader = Shader(SHADER_DIR"/preview/point.vs", SHADER_DIR"/preview/point.fs");
    std::string previewModelDir = MODEL_DIR"/../../09-cubemap/models/skybox/skybox.obj";
    depthMap = new Model(previewModelDir, previewShader, mat4(1), GL_CLAMP_TO_EDGE, false);
    //t1.shouldUse = false;
    depthMap->setTextures({t2});
    depthMap->shader.use();
    depthMap->bindUniformBlock("ViewProjection", BINDING_POINT_VP);
#endif
}

void Renderer::render(Scene& scene, const Camera *camera) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vBuffer.update(0, camera->getViewMatrix());
    vBuffer.update(1, config::projectionMatrix(camera->fov));
    
    auto dirLights = scene.getLights<Sun>();
    // 除非没有平面物体产生阴影的需求 否则不要用这个！
#if USE_FRONT_CULLING
    glCullFace(GL_FRONT);
#endif
    for (int i = 0; i < dirShadowMaps.size(); i++) {
        dirShadowMaps[i].activate();
        glClear(GL_DEPTH_BUFFER_BIT);
        render1(scene, 0, &dirLights[i]->Light::shader);
        dirShadowMaps[i].deactivate();
    }
    auto pointLights = scene.getLights<Bulb>();
    for (int i = 0; i < pointShadowMaps.size(); i++) {
        pointShadowMaps[i].activate();
        // 这里 dir map 不 clear 还能显示，但 point 会黑屏（全0）？
        glClear(GL_DEPTH_BUFFER_BIT);

        render1(scene, 0, &pointLights[i]->Light::shader);
        pointShadowMaps[i].deactivate();
    }
#if USE_FRONT_CULLING
    glCullFace(GL_BACK);
#endif
    
    glViewport(0, 0, screenPixelW, screenPixelH);
 
#if USE_DEPTH_PREVIEW || USE_CUBE_PREVIEW
    glFrontFace(GL_CW);
    depthMap->draw();
    glFrontFace(GL_CCW);
#else
    render1(scene, camera);
#endif
}

/// 渲染 models
void Renderer::render1(Scene& scene, const Camera *camera, const Shader *customShader) {
    for (int i = 0; i < scene.models.size(); i++) {
        if (!customShader) {
            scene.modelAt(i).draw();
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
    dirLightBuffer(dirLightMat4Count * dirLightCount, GL_STATIC_DRAW),
    pointLightBuffer(pointLightMat4Count * pointLightCount, GL_STATIC_DRAW),
    cubeMatrices(6, GL_STATIC_DRAW)
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);
    // 我觉得 cull 效果反而不好，不如 bias？
    glEnable(GL_CULL_FACE);

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
        model.get()->bindUniformBlock("PointLights", BINDING_POINT_POINT_LIGHTS);
        model.get()->shader.setFloat("farPlane", pointShadowFarPlane);
    }
    vBuffer.bindRange(0, 2 * sizeof(mat4), BINDING_POINT_VP);
    dirLightBuffer.bindRange(0, dirLightMat4Count * sizeof(mat4), BINDING_POINT_DIR_LIGHTS);
    pointLightBuffer.bindRange(0, pointLightMat4Count * sizeof(mat4), BINDING_POINT_POINT_LIGHTS);
    
    setup(scene);
}
