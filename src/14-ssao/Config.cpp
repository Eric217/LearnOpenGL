//
//  Config.cpp
//  08-framebuffer
//
//  Created by Eric on 2022/10/22.
//

#include "Config.hpp"
#include "Model.hpp"
#include "Camera.h"
#include "Scene.hpp"

#include <glad/glad.h>
#include <vector>
#include <glm/ext.hpp>

using namespace config;
using namespace glm;

int config::initWindowPointSizeW = 700;
int config::initWindowPointSizeH = 560;

int config::screenPixelW = 0;
int config::screenPixelH = 0;

float config::cameraStep = 6;
float config::cameraFarPlane = 120;

bool config::usingHDR = 1;
bool config::usingSRGB = true;
bool config::using_GL_SRGB = true;
bool config::usingBloom = 1;
bool config::usingDeferred = 1;
bool config::debugShowLightVolume = 0;
bool config::usingDeferredAo = 1;

Scene config::loadScene() {
    const std::string cubeDir = GLOBAL_MODEL_DIR"/cube/cube.obj";
    const std::string containerDir = GLOBAL_MODEL_DIR"/container/cube.obj";
    const std::string container2Dir = GLOBAL_MODEL_DIR"/container/cube2.obj";
    const std::string nanoDir = GLOBAL_MODEL_DIR"/nanosuit/nanosuit.obj";
    const std::string quadDir = GLOBAL_MODEL_DIR"/quad/quad.obj";
    const std::string floorDir = GLOBAL_MODEL_DIR"/floor/floor.obj";
    const std::string ballDir = GLOBAL_MODEL_DIR"/ball/ball.obj";

    const std::string shaderDir = SHADER_DIR;

    Scene scene;
    const mat4 id4(1.f);

    // cubes
    {
        auto shader = Shader(shaderDir + "/object/v.vs", shaderDir + "/object/f.fs");
        // 在屏幕上分别是 大中小
        vec3 positions[3] = { {-8, 7, -2.1},   {0, 1.3001, 3},  {3,   0.1003, 0  }};
        vec3 scales[3] =    { {4.8, 8, 0.5},   {1,  2.3, 1},    {1.2, 1.1,    1.2}};
        for (int i = 0; i < 3; i++) {
            scene.addModel(i == 0 ? container2Dir : containerDir,
                           scale(translate(id4, positions[i]), scales[i]),
                           shader);
        }
    }
    // floor
    {
        auto shader = Shader(shaderDir + "/object/v.vs", shaderDir + "/object/f.fs");
        float scaleF = 3;
        auto m = new Model(floorDir, shader, scale(id4, vec3(scaleF, 1, scaleF)));
        scene.addModel(m);
    }
    // nano
    {
        auto shader = Shader(SHADER_DIR "/object/v.vs", SHADER_DIR "/object/f.fs", false);
        auto matT = translate(id4, vec3(-8.1,-1.18,-4))
                    * rotate(id4, radians(180.f), vec3(0,1,0))
                    * scale(id4, vec3(1));
        auto m = new Model(nanoDir, shader, matT);
        scene.addModel(m);
    }
    // lights
    {
        vec3 pos = 5.f * vec3(5, -7.3, -8);
        vec3 dir = normalize(pos);
        auto mat = translate(id4, -pos) *
                    scale(id4, vec3(9, 9, 9));
        auto lightShader = Shader(shaderDir + "/light/v.vs", shaderDir + "/light/f.fs", false);
        auto shadowShader = Shader(shaderDir + "/shadow/dir.vs", shaderDir + "/shadow/dir.fs", false);

        Sun *sunPtr = new Sun(cubeDir, lightShader, mat);
        
        auto &light = *sunPtr;
        light.name = "dirLights";
        light.direction = dir;
        light.ambient = vec3(255, 198, 107)/255.f * 0.275f;
        light.diffuse = light.ambient;
        light.specular = light.ambient;
        light.Light::shader = shadowShader;
        
        std::shared_ptr<Sun> ptr(sunPtr);
        scene.addLight(ptr);
        scene.addModel(ptr);
    }
    vec3 bulbTrans[2] = {
        vec3(3.59f, 1.8f, -2.45f),
        vec3(-8.2f, 13.53f, -5.4f), // nano
    };
    
    std::vector<std::shared_ptr<Model>> volumes;
    
    for (int i = 0; i < 2; i++) {
        auto mat = translate(id4, bulbTrans[i]);
        mat *= scale(id4, vec3(0.1f, 0.1f, 0.1f));
        auto lightShader = Shader(shaderDir + "/light/v.vs", shaderDir + "/light/f.fs", false);
        auto shadowShader = Shader(shaderDir + "/shadow/p.vs", shaderDir + "/shadow/p.fs", shaderDir + "/shadow/p.gs", false);

        Bulb *bulbPtr = new Bulb(cubeDir, mat);
        bulbPtr->setTextures({});
        
        auto &bulb = *bulbPtr;
        bulb.Model:: shader = lightShader;
        bulb.Light:: shader = shadowShader;
        
        float rate = 1;
        bulb.name = "pointLights";
        bulb.ambient = rate * vec3(i == 0 ? 21 : 28);
        bulb.diffuse = bulb.ambient;
        bulb.specular = bulb.ambient;
        bulb.k0 = 1;
        bulb.k1 = 0.001;
        bulb.k2 = i == 0 ? 9 : 11;
        bulb.r = bulb.lightVolumeRadius();
        
        std::shared_ptr<Bulb> ptr(bulbPtr);
        scene.addLight(ptr);
        scene.addModel(ptr);
        
        // 110 是模型的半径
        auto t = translate(id4, bulb.position) * scale(id4, vec3(bulb.r / 110));
        if (debugShowLightVolume) {
            auto shader = Shader(SHADER_DIR"/volume/preview.vs",
                                 SHADER_DIR"/volume/preview.fs");
            scene.addModel(ballDir, t, shader);
        } else {
            if (usingDeferred) {
                volumes.push_back(std::make_shared<Model>(ballDir, 0, t));
            }
        }
    }
    {
        auto shader = Shader(SHADER_DIR"/hdr/v.vs", SHADER_DIR"/hdr/f.fs");
        auto model = new Model(quadDir, shader, id4, GL_CLAMP_TO_EDGE, false);
        scene.setHdrQuad(model);
    }
    if (usingBloom) {
        scene.setBloomShader(Shader(SHADER_DIR"/bloom/v.vs",
                                    SHADER_DIR"/bloom/f.fs"));
    }
    if (usingDeferred) {
        const std::string s = SHADER_DIR "/deferred/";
        auto quadShader = Shader(s + "quad/v.vs", s + "quad/f.fs");
        auto data = new DeferredShadingData(
                            Model(quadDir, quadShader, id4, GL_CLAMP_TO_EDGE),
                            Shader(s + "mrt/v.vs", s + "mrt/f.fs"));
        data->volumes = std::move(volumes);
        data->volumeShader = Shader(s + "volume/v.vs", s + "volume/f.fs");
        scene.setDeferredData(data);
    }
    return scene;
}

mat4 config::projectionMatrix(float fovDegree) {
    return glm::perspective(radians(fovDegree), 1.f * screenPixelW / screenPixelH, 0.1f, cameraFarPlane);
}

mat4 config::perspective(float fovDegree, float whRatio, float near, float far) {
    return glm::perspective(radians(fovDegree), whRatio, near, far);
}

Camera* config::loadCamera() {
    auto camera = new Camera();
    camera->position = vec3(0.f, 4.5f, 8.3f);
    camera->front = normalize(vec3(0.f, 0.f, -1.f));
    camera->upVec = vec3(0.f, 1.f, 0.f);
    camera->fov = 38;
    return camera;
}

void config:: restoreDefaultViewport() {
    glViewport(0, 0, screenPixelW, screenPixelH);
}

#if NDEBUG
#else
std::map<int, size_t> EfficiencyTool::TIMES;
std::map<int, size_t> EfficiencyTool::TOTAL;
#endif
