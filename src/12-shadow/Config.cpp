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
float config::cameraFarPlane = 200;

bool config::usingHDR = true;
bool config::usingSRGB = true;
bool config::using_GL_SRGB = true;
bool config::usingBloom = true;

Scene config::loadScene() {
    const std::string cubeDir = MODEL_DIR"/cube/cube.obj";
    const std::string floorDir = MODEL_DIR"/floor/floor.obj";
    const std::string shaderDir = SHADER_DIR;

    Scene scene;
    const mat4 id4(1.f);
  
    // cubes
    {
        auto shader = Shader(shaderDir + "/object/v.vs", shaderDir + "/object/f.fs");
        // 在屏幕上分别是 3, 1, 2
        vec3 positions[3] = { {2, -0.1501, -4.6},    {-3, 1.3001, 3}, {0, 0.1003, 0} };
        vec3 scales[3] =    { {1.03, 0.85, 1.03}, {1, 2.3, 1},{1.1, 1.1, 1.1}};
        for (int i = 0; i<3; i++) {
            scene.addModel(cubeDir,
                scale(translate(id4, positions[i]), scales[i]),
                shader);
        }
    }
    // floor
    {
        auto shader = Shader(shaderDir + "/object/v.vs", shaderDir + "/object/f.fs");
        auto m = new Model(floorDir, shader, scale(id4, vec3(1.15, 1, 1.15)));
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
        sunPtr->setTextures({});
        
        auto &light = *sunPtr;
        light.name = "dirLights";
        light.direction = dir;
        light.ambient = vec3(255, 198, 107)/255.f * 0.66f;
        light.diffuse = light.ambient;
        light.specular = light.ambient;
        light.Light::shader = shadowShader;
        
        std::shared_ptr<Sun> ptr(sunPtr);
        scene.addLight(ptr);
        scene.addModel(ptr);
    }
    {
#define ENABLE_POINT_LIGHT 1
#if ENABLE_POINT_LIGHT
        auto mat = translate(id4, vec3(0.59f, 1.8f, -2.45f));
        mat *= scale(id4, vec3(0.1f, 0.1f, 0.1f));
        auto lightShader = Shader(shaderDir + "/light/v.vs", shaderDir + "/light/f.fs", false);
        auto shadowShader = Shader(shaderDir + "/shadow/p.vs", shaderDir + "/shadow/p.fs", shaderDir + "/shadow/p.gs", false);

        Bulb *bulbPtr = new Bulb(cubeDir, mat);
        bulbPtr->setTextures({});
        
        auto &bulb = *bulbPtr;
        bulb.Model:: shader = lightShader;
        bulb.Light:: shader = shadowShader;
        
        bulb.name = "pointLights";
        bulb.ambient = vec3(21);
        bulb.diffuse = vec3(21);
        bulb.specular = vec3(21);
        bulb.k0 = 1;
        bulb.k1 = 0.00004;
        bulb.k2 = 0.000006;

        std::shared_ptr<Bulb> ptr(bulbPtr);
        scene.addLight(ptr);
        scene.addModel(ptr);
#endif
    }
    {
        auto shader = Shader(SHADER_DIR"/hdr/v.vs", SHADER_DIR"/hdr/f.fs");
        auto model = new Model(
            MODEL_DIR"/../../08-framebuffer/models/quadr/quadr.obj",
            shader, id4, GL_CLAMP_TO_EDGE, false);
        scene.setHdrQuad(model);
    }
    if (usingBloom) {
        scene.setBloomShader(Shader(SHADER_DIR"/bloom/v.vs",
                                    SHADER_DIR"/bloom/f.fs"));
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
