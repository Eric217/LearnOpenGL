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
#include <random>
#include <glm/ext.hpp>

using namespace glm;
using namespace config;

#define BINDING_POINT_VP 0
#define BINDING_POINT_DIR_LIGHTS 1
#define BINDING_POINT_POINT_LIGHTS 2
#define BINDING_POINT_CUBE_MAT 4
#define BINDING_POINT_CONTEXT 3
#define BINDING_POINT_CUBE_MAT2 5
#define BINDING_POINT_AO_CONFIG 6
#define BINDING_POINT_CAMERA_CONFIG 7

constexpr int dirLightCount = 1;
constexpr int dirLightMat4Count = 2;

constexpr int pointLightCount = 2;
constexpr int pointLightMat4Count = 2;

constexpr float pointShadowFarPlane = 30;
constexpr int bloomBlurCount = 9;
constexpr float ppBufferScale = 0.2;

#define DepthFrameBufferSize 1024

/// 预览用的几个局部变量/宏
static Model *depthMap;

#define USE_BLOOM_PREVIEW 0 // 看高斯模糊的效果
#define USE_DEPTH_PREVIEW 0 // 看平行光的阴影图
#define USE_CUBE_PREVIEW 0 // 预览第一个点光源的阴影图
#define USE_CUBE_PREVIEW_1 0 // 预览第二个点光源的阴影图
#define USE_FRONT_CULLING 0 // 要废弃了..效果不好
#define USE_HDR_COMPARING 0 // 对比 HDR 开关效果
#define USE_AO_RAW_PREVIEW 0 // 看 AO 的效果
#define USE_AO_BLUR_PREVIEW 0 // 看 AO 的效果

void Renderer::render(Scene& scene, const Camera *camera) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // 更新一些基础全局变量
    vBuffer.update(0, camera->getViewMatrix());
    vBuffer.update(1, config::projectionMatrix(camera->fov));
    context.updateCameraExposure(bool(usingHDR) * camera->exposure);
    
    // 光源拍照 用于计算阴影
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
    restoreDefaultViewport();
    
    if (debugShowLightVolume) {
        render1(scene, camera);
        return;
    }
    // 阴影预览
#if USE_DEPTH_PREVIEW || USE_CUBE_PREVIEW || USE_CUBE_PREVIEW_1
    glFrontFace(GL_CW);
    depthMap->draw();
    glFrontFace(GL_CCW);
#else
    // 渲染世界
    // 如果不开 HDR、Bloom、Deferred，直接 render1 渲染到屏幕即可
    if (!usingHDR && !usingBloom && !usingDeferred) {
        render1(scene, camera);
    } else if (!usingDeferred) {
        render2(scene, camera);
    } else {
        render3(scene, camera);
    }
#endif
}

const GLenum usingBuffers3[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
    GL_COLOR_ATTACHMENT2};
const GLenum usingBuffers2[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
static GLuint renderBlur(GLuint texId, const Model &quad, PingPongBuffer &ppBuffer, const Shader &gaussionFilter, int blurCount);

/// 延迟管线渲染
void Renderer::render3(Scene& scene, const Camera *camera) {
    // G-buffer pass
    auto &deferBuffer = getDeferredBuffer();
    deferBuffer.activate();
    // 把世界画到 3 个纹理，pos+z，normal3，color4
    glDrawBuffers(3, usingBuffers3);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    render1(scene, camera, &scene.getDeferredMRTShader());
    deferBuffer.deactivate();
    
    // ao pass
    if (usingDeferredAo) {
        tmpQuad.get()->draw(aoDrawer, aoPassTex, *aoMap, GL_DEPTH_BUFFER_BIT);
#if USE_AO_RAW_PREVIEW
        tmpQuad.get()->draw(depthDrawer, {Texture(aoMap.get()->asTexture(), diffuse)});
        return;
#endif
        auto ID = renderBlur(aoMap.get()->asTexture(), *tmpQuad, *blurBuffer, scene.gaussionFilter, 2);
#if USE_AO_BLUR_PREVIEW
        tmpQuad.get()->draw(depthDrawer, {Texture(ID, diffuse)});
        return;
#endif
        aoTex[0].ID = ID;
    }

    auto &canvasBuffer = *tmp2Buffer.get();
    auto &volumeBuffer = *tmpBuffer.get();
    canvasBuffer.activate();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawBuffers(2, usingBuffers2); // bloom

    // 直接绘制 quad 虽然可以直接计算所有光源着色，但无法用 light volume 优化点光源
    auto &quad = scene.getDeferredQuad();
    if (usingDeferredAo) {
        quad.appendUniqueTextures(aoTex);
    }
    // 因此这里只着色 dir light，一会画 light volume
    quad.draw();
    // 至此就可以显示 canvas 看延迟管线的效果了！下面开始用优化的方式进行点光源着色
    canvasBuffer.deactivate();
    
    // 光体积可见的像素进行点光源着色
    // 为了提高性能，球只画一面，但是相机进入球内会丢掉画面，可以临时改为 cull front
    glCullFace(GL_FRONT);
    // 因为要和延迟管线的 dir light 图合并，我们可以使用 blend 或两个纹理进行采样
    // 两次在同一个 buffer 里 draw + blend 有问题：
    // 某次 draw 后续可能因为深度大被丢弃，但颜色已经 blend 进去了！必须单独开 buffer 画一次
    volumeBuffer.activate();
    glDrawBuffers(2, usingBuffers2); // bloom
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto &volumes = scene.getLightVolumes();
    for (int i = 0; i < volumes.size(); i ++) {
        volumes[i].get()->draw(scene.getPointLightShader());
    }
    volumeBuffer.deactivate();
    glCullFace(GL_BACK);
    // canvas(tmp2) 和 tmp 各存了一部分 bloom
    
    // blend 到 canvas，1+1 模式
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    canvasBuffer.activate();
    tmpQuad.get()->draw(image2Drawer);
    glDisable(GL_BLEND);
    // 现在 canvas 已有深度为 1（两次 quad），color、bloom 都 blend 来了
    
    // lights redraw，需要 blit 深度，画 lights，用灯本身的颜色着色
    // blit 传送的 src、dst 分别是当前绑定的 GL_READ_FRAMEBUFFER、GL_DRAW_FRAMEBUFFER
    // 这里有一个隐藏问题，canvas 看起来可以换成是屏幕，但是屏幕的深度缓存位数或格式不同，
    // 导致我们自己的 DEPTH32/24 的 buffer blit 过去之后损失精度。
    // 这个时候用失真的 z-buffer 画灯，会有 z-fighting，所以只能全画到 canvas
    deferBuffer.bindToTarget(GL_READ_FRAMEBUFFER);
    canvasBuffer.bindToTarget(GL_DRAW_FRAMEBUFFER);
    glBlitFramebuffer(0, 0, deferBuffer.w, deferBuffer.h,
                      0, 0, canvasBuffer.w, canvasBuffer.h,
                      GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    // 现在 canvas 是所有物体的正确深度，开始重新画灯，同时写入 bloom
    for (auto &light: scene.getLights()) {
        auto model = dynamic_cast<Model*>(light.get());
        if (model) {
            model->draw();
        }
    }
    canvasBuffer.deactivate();
    // 现在场景颜色都保存在 canvasBuffer 中，如果 blit 到屏幕，则不会伽马校正！
/*  // 所以用以下代码没有 SRGB 显示起来巨暗！
    canvasBuffer.bindToTarget(GL_READ_FRAMEBUFFER);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, canvasBuffer.w, canvasBuffer.h,
                      0, 0, screenPixelW, screenPixelH,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
  */
    if (usingHDR) {
        renderHDR(scene, camera, canvasBuffer);
    } else {
        // 这里会失真严重
        // 因为 mrt 肯定是 32 位，虽然颜色可以做 8 位，但我偷懒，给颜色也统一用了 32
        // 现在 canvas 是 8 位，导致画上去失真
        renderImage(canvasBuffer.getTextureAt(0));
    }
}

void Renderer::renderImage(GLuint id) const {
    tmpQuad.get()->draw(imageDrawer, {Texture(id, diffuse)});
}

void Renderer::renderHDR(Scene& scene, const Camera *camera, const MRTNormalBuffer &buffer) {
    auto &quad = scene.getHdrQuad();
    if (!usingBloom) {
        quad.draw({Texture(buffer.getTextureAt(0), diffuse)});
        return;
    }
    // 现在把光源图进行高斯模糊，优化后的高斯算法需要使用 pingpong buffer 提高效率
    auto baseTex = Texture(buffer.getTextureAt(0), diffuse);
    auto lightTex = Texture(buffer.getTextureAt(1), diffuse);
    auto &pFrameBuffer = *ppBuffer.get();
    auto &shader = scene.gaussionFilter;
    
    // 第一步，把 光源图 渲染到 ping buffer
    // 第二步，把 ping buffer 图 渲染到 pong buffer
    // 需要循环进行第 1、2 步 N 次
    quad.setTextures({lightTex});
    shader.use();
    // 为了性能，ppBuffer 是很小的 viewport
    for (int i = 0; i < bloomBlurCount * 2; i++) {
        pFrameBuffer.activate(i % 2);
        glClear(GL_COLOR_BUFFER_BIT);
        shader.setBool("usingPing", (i + 1) % 2);
        quad.draw(shader);
        quad.updateTexture(pFrameBuffer.asTexture());
    }
    pFrameBuffer.deactivate();
    restoreDefaultViewport();
    
#if USE_BLOOM_PREVIEW
    // 已取出 pong 里的纹理图，diffuse 渲染到屏幕
    // 只有一个模糊后的光源小图
#else
    // 合成
    quad.setTextures({
        baseTex, // 基础图
        Texture(pFrameBuffer.asTexture(), bloom) // bloom
    });
#endif
    quad.draw();
}

/// HDR & Bloom
void Renderer::render2(Scene& scene, const Camera *camera) {
    // 把世界离屏渲染到 hdr buffer，同时多目标渲染一个光源图
    getHdrBuffer().activate();
    if (usingBloom) {
        glDrawBuffers(2, usingBuffers2);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    render1(scene, camera);
    getHdrBuffer().deactivate();
     
    renderHDR(scene, camera, getHdrBuffer());
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

static GLuint renderBlur(GLuint texId, const Model &quad, PingPongBuffer &ppBuffer, const Shader &gaussionFilter, int blurCount) {
    auto &shader = gaussionFilter;
    shader.use();

    // 第一步，把 图 渲染到 ping buffer
    // 第二步，把 ping buffer 图 渲染到 pong buffer
    // 需要循环进行第 1、2 步 blurCount 次
    std::vector<Texture> vec = {Texture(texId, diffuse)};
    
    for (int i = 0; i < blurCount * 2; i++) {
        ppBuffer.activate(i % 2);
        glClear(GL_COLOR_BUFFER_BIT);
        shader.setBool("usingPing", (i + 1) % 2);
        quad.draw(shader, vec);
        vec[0].ID = ppBuffer.asTexture();
    }
    ppBuffer.deactivate();
    return vec[0].ID;
}

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
        float frustum = 28.f; // 视锥外面都视为无阴影
        auto pv = ortho(-frustum, frustum, -frustum, frustum, 0.1f, config::cameraFarPlane) * v;
        light.Light::shader.setMat4("vpMatrix", pv);
        dirLightBuffer.update(1 + 2 * i, pv);
        
        auto &models = scene.models;
        Texture t(b.asTexture(), dirlight);
        auto vec = {t};
        t1 = t;
        for (auto &model: models) {
            model.get()->appendTextures(vec);
        }
        if (usingDeferred) {
            scene.getDeferredQuad().appendTextures(vec);
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
        light.shader.bindUniformBlock("Transforms", BINDING_POINT_CUBE_MAT + i);
        UniformBufferM4 cubeM(6, GL_STATIC_DRAW);
        cubeM.bindRange(0, 6 * sizeof(mat4), BINDING_POINT_CUBE_MAT + i);
        cubeMatrices.push_back(cubeM);
         
        model.shader.use();
        model.shader.setVec3("lightColor", light.ambient);
        
        auto b = CubeDepthFramebuffer(DepthFrameBufferSize, DepthFrameBufferSize);
        pointShadowMaps.push_back(b);
        
        Texture t(b.asTexture(), pointlight);
#if USE_CUBE_PREVIEW_1
        if (i == 1) {
            t2 = t;
        }
#endif
#if USE_CUBE_PREVIEW
        if (i == 0) {
            t2 = t;
        }
#endif
        auto vec = {t};
        auto &models = scene.models;
        for (auto &model: models) {
            model.get()->appendTextures(vec);
        }
        if (usingDeferred) {
            auto &volumes = scene.getLightVolumes();
            for (auto &volume: volumes) {
                volume.get()->appendTextures(vec);
            }
        }
        pointLightBuffer.update(0 + 4 * pointLightMat4Count * i, light.position);
        pointLightBuffer.update(1 + 4 * pointLightMat4Count * i, light.ambient);
        pointLightBuffer.update(2 + 4 * pointLightMat4Count * i, light.diffuse);
        pointLightBuffer.update(3 + 4 * pointLightMat4Count * i, light.specular);
        pointLightBuffer.update(4 + 4 * pointLightMat4Count * i,
                                vec4(light.k0, light.k1, light.k2, light.r));
 
        mat4 proj = config::perspective(90, 1, 0.5, pointShadowFarPlane);
        // 关于这里 lookat 的 up vec，因为是给 cubemap 用的，
        // cubemap 会假设纹理是从外面看的样子，他内部会有 1-x 的操作，
        // 所以我们绘制一个面的时候要反着画!
        cubeM.update(0, proj * lookAt(light.position, light.position + vec3(1,0,0), vec3(0,-1,0)));
        cubeM.update(1, proj * lookAt(light.position, light.position + vec3(-1,0,0), vec3(0,-1,0)));
        cubeM.update(2, proj * lookAt(light.position, light.position + vec3(0,1,0), vec3(0,0,1)));
        cubeM.update(3, proj * lookAt(light.position, light.position + vec3(0,-1,0), vec3(0,0,-1)));
        cubeM.update(4, proj * lookAt(light.position, light.position + vec3(0,0,1), vec3(0,-1,0)));
        cubeM.update(5, proj * lookAt(light.position, light.position + vec3(0,0,-1), vec3(0,-1,0)));
    }
    
#if USE_DEPTH_PREVIEW
    auto previewShader = Shader(SHADER_DIR"/preview/dir.vs", SHADER_DIR"/preview/dir.fs");
    std::string previewModelDir = MODEL_DIR"/../../08-framebuffer/models/quadr/quadr.obj";
    depthMap = new Model(previewModelDir, previewShader, mat4(1), GL_CLAMP_TO_EDGE, false);
    t2.shouldUse = false;
    depthMap->setTextures({t1});
#elif USE_CUBE_PREVIEW || USE_CUBE_PREVIEW_1
    auto previewShader = Shader(SHADER_DIR"/preview/point.vs", SHADER_DIR"/preview/point.fs");
    std::string previewModelDir = MODEL_DIR"/../../09-cubemap/models/skybox/skybox.obj";
    depthMap = new Model(previewModelDir, previewShader, mat4(1), GL_CLAMP_TO_EDGE, false);
    //t1.shouldUse = false;
    depthMap->setTextures({t2});
    depthMap->shader.use();
    depthMap->bindUniformBlock("ViewProjection", BINDING_POINT_VP);
#endif
}

void Renderer::updateScreenSize(int w, int h) {
    glViewport(0, 0, w, h);
    
    if (hdrBuffer) {
        hdrBuffer.get()->updateSize(w, h);
    }
    if (ppBuffer) {
        ppBuffer.get()->updateSize(w * ppBufferScale, h * ppBufferScale);
    }
    if (deferredBuffer) {
        deferredBuffer.get()->updateSize(w, h);
    }
    if (aoMap) {
        aoMap.get()->updateSize(w, h);
    }
    tmpBuffer.get()->updateSize(w, h);
    tmp2Buffer.get()->updateSize(w, h);

    context.updateScreenSize(w, h);
}

Renderer::Renderer(const Scene &scene):
    vBuffer(2, GL_STREAM_DRAW),
    context(BINDING_POINT_CONTEXT),
    aoConfig(BINDING_POINT_AO_CONFIG),
    cameraConfig(BINDING_POINT_CAMERA_CONFIG),
    dirLightBuffer(dirLightMat4Count * dirLightCount, GL_STATIC_DRAW),
    pointLightBuffer(pointLightMat4Count * pointLightCount, GL_STATIC_DRAW),
    aoDrawer(SHADER_DIR"/deferred/ao/v.vs", SHADER_DIR"/deferred/ao/f.fs"),
    imageDrawer(SHADER_DIR"/preview/image.vs", SHADER_DIR"/preview/image.fs"),
    image2Drawer(SHADER_DIR"/preview/image.vs", SHADER_DIR"/preview/2image.fs"),
    depthDrawer(SHADER_DIR"/preview/image.vs", SHADER_DIR"/preview/depth.fs"),
    deferZDrawer(SHADER_DIR"/preview/image.vs", SHADER_DIR"/preview/deferZ.fs"),
    noiseDrawer(SHADER_DIR"/preview/image.vs", SHADER_DIR"/preview/noise.fs"),
    tmpQuad(new Model(GLOBAL_MODEL_DIR"/quad/quad.obj", 0, mat4(1), GL_CLAMP_TO_EDGE))
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL); // 让 quad 深度为 1 时可以显示
    
    if (usingSRGB && using_GL_SRGB) {
        glEnable(GL_FRAMEBUFFER_SRGB);
    }
    // 我觉得 cull back 阴影效果反而不好，不如 bias？
    glEnable(GL_CULL_FACE);

    glClearColor(0, 0, 0, 1.f);

    
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
    pointLightBuffer.bindRange(0, pointLightMat4Count * pointLightCount * sizeof(mat4), BINDING_POINT_POINT_LIGHTS);
    {
        auto &quad = scene.getHdrQuad();
        auto texId = getHdrBuffer().asTexture();
        quad.setTextures({Texture(texId, diffuse)});
        quad.shader.use();
        quad.bindUniformBlock("Context", BINDING_POINT_CONTEXT);
        quad.shader.setBool("hdrMask", USE_HDR_COMPARING);
    }
    if (usingBloom) {
        ppBuffer.reset(new PingPongBuffer(screenPixelW * ppBufferScale, screenPixelW * ppBufferScale, usingHDR));
    }
    context.updateScreenSize(screenPixelW, screenPixelH);
    context.updateFarPlane_ps(pointShadowFarPlane);
    cameraConfig.updateFarPlane(cameraFarPlane);
    
    if (usingDeferred) {
        // 一个 world pos(z as alpha)，一个 normal，一个 diffuse + specular(spec as alpha channel)
        int channels[] = {4, 3, 4};
        // 这个 buffer 必须 32 位，因为不是纯粹的颜色，而是点/向量
        auto p = new MRTNormalBuffer(screenPixelW, screenPixelH, 3, 1, channels, 0);
        deferredBuffer.reset(p);
        
        deferredTex = {
            Texture(p->getTextureAt(0), deferPos),
            Texture(p->getTextureAt(1), deferNormal),
            Texture(p->getTextureAt(2), deferColor),
        };
        aoTex.push_back(Texture(0, TextureType::aoMap, usingDeferredAo));
        if (usingDeferredAo) {
            std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
            std::default_random_engine generator;
            
            aoPassTex.insert(aoPassTex.end(), deferredTex.begin(), deferredTex.begin() + 2);
            {
                std::vector<vec3> noise;
                float test = 1 ? randomFloats(generator) * 2 - 1 : 0;
                for (int i = 0; i < 4 * 4; i ++) {
                    noise.push_back(vec3(randomFloats(generator) * 2 - 1,
                                         randomFloats(generator) * 2 - 1,
                                         test));
                }
                // 采样时，用 coor * size / 4
                aoPassTex.push_back(RepeatingTexture(4, 4, &noise[0]));
            }
            aoMap.reset(new DepthFramebuffer(screenPixelW, screenPixelH));
            
            aoDrawer.use();
            aoDrawer.bindUniformBlock("ViewProjection", BINDING_POINT_VP);
            aoDrawer.bindUniformBlock("AO_Config", BINDING_POINT_AO_CONFIG);
             
            std::vector<vec4> samples;
            for (int i = 0; i < 64; i++) {
                samples.push_back(normalize(vec4(
                    randomFloats(generator) * 2 - 1,
                    randomFloats(generator) * 2 - 1,
                    randomFloats(generator), 1
                )));
            }
            aoConfig.updateSamples(samples);
            aoConfig.updateGather(2.2);
            aoConfig.updateRadius(1.1);
            aoConfig.updateIntensity(1);
            aoConfig.updateKernelSize(64);
            aoConfig.updateAttenuation(0.3); // 0.22
            
            blurBuffer.reset(new PingPongBuffer(screenPixelW, screenPixelW, usingHDR));
        }
        auto &quad = scene.getDeferredQuad();
        quad.setTextures({});
        quad.appendTextures(deferredTex);
        quad.prepareDrawing();
        quad.bindUniformBlock("DirLights", BINDING_POINT_DIR_LIGHTS);
        quad.bindUniformBlock("ViewProjection", BINDING_POINT_VP);

        scene.getDeferredMRTShader().use();
        scene.getDeferredMRTShader().bindUniformBlock("ViewProjection", BINDING_POINT_VP);
        
        auto &volumes = scene.getLightVolumes();
        for (int i = 0; i < volumes.size(); i ++) {
            volumes[i].get()->setTextures({});
            volumes[i].get()->appendTextures(deferredTex);
        }
        scene.getPointLightShader().use();
        scene.getPointLightShader().bindUniformBlock("Context", BINDING_POINT_CONTEXT);
        scene.getPointLightShader().bindUniformBlock("PointLights", BINDING_POINT_POINT_LIGHTS);
        scene.getPointLightShader().bindUniformBlock("ViewProjection", BINDING_POINT_VP);
        
    }
    tmpBuffer.reset(new MRTNormalBuffer(screenPixelW, screenPixelH, 2, usingHDR, 0, 0));
    tmp2Buffer.reset(new MRTNormalBuffer(screenPixelW, screenPixelH, 2, usingHDR, 0, 0));
    
    tmpQuad.get()->setTextures({
        Texture(tmpBuffer.get()->getTextureAt(0), diffuse),
        Texture(tmpBuffer.get()->getTextureAt(1), diffuse), // bloom 用的
    });
    
    setup(scene);
}

const MRTNormalBuffer& Renderer::getHdrBuffer() {
    if (hdrBuffer) {
        return *hdrBuffer;
    }
    hdrBuffer = std::shared_ptr<MRTNormalBuffer>(
        new MRTNormalBuffer(screenPixelW, screenPixelH, 2, usingHDR, 0, 0));
    return *hdrBuffer;
}

const MRTNormalBuffer& Renderer::getDeferredBuffer() {
    return *deferredBuffer;
}
