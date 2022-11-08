//
//  Config.cpp
//  08-framebuffer
//
//  Created by Eric on 2022/10/22.
//

#include "Config.hpp"
#include "Model.hpp"
#include "Camera.h"

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

Scene config::loadScene() {
    const std::string cubeDir = MODEL_DIR"/cube/cube.obj";
    const std::string floorDir = MODEL_DIR"/floor/floor.obj";
    const std::string shaderDir = SHADER_DIR;

    Scene scene;
    mat4 id4(1.f);
  
    // cubes
    {
        auto shader = Shader(shaderDir + "/object/v.vs", shaderDir + "/object/f.fs");
        vec3 positions[3] = { {2, 1.001, -4},    {-3, 0.002, 3}, {0, 0.33, 0} };
        vec3 scales[3] =    { {1.2, 2, 1.25}, {1, 1, 1},{1.3, 1.3, 1.3}};
        for (int i = 0; i<3; i++) {
            scene.addModel(cubeDir,
                scale(translate(id4, positions[i]), scales[i]),
                shader);
        }
    }
    // floor
    {
        auto shader = Shader(shaderDir + "/object/v.vs", shaderDir + "/object/f.fs");
        auto m = new Model(floorDir, shader);
        scene.addModel(m);
    }
    // lights
    {
        vec3 pos = 10.f * vec3(5, -7.3, -8);
        vec3 dir = normalize(pos);
        auto mat = translate(id4, -pos) *
                    scale(id4, vec3(10.f, 10.f, 10.f));
        auto lightShader = Shader(shaderDir + "/light/v.vs", shaderDir + "/light/f.fs");
        auto shadowShader = Shader(shaderDir + "/shadow/dir.vs", shaderDir + "/shadow/dir.fs");

        Sun *sunPtr = new Sun(cubeDir, lightShader, mat);
        auto &light = *sunPtr;
        light.name = "dirLights";
        light.direction = dir;
        light.ambient = vec3(255, 198, 107)/255.f;
        light.diffuse = light.ambient;
        light.specular = vec3(1);
        light.Light::shader = shadowShader;
        
        std::shared_ptr<Sun> ptr(sunPtr);
        scene.addLight(ptr);
        scene.addModel(ptr);
    }
    {
#define ENABLE_POINT_LIGHT 0
#if ENABLE_POINT_LIGHT
        auto mat2 = translate(id4, vec3(0.5f, 5.f, 1.7f));
        mat2 *= scale(id4, vec3(0.3f, 0.3f, 0.3f));
        auto lightShader = Shader(shaderDir + "/light/v.vs", shaderDir + "/light/f.fs");

        Bulb *bulbPtr = new Bulb(cubeDir, mat2);
        auto &bulb = *bulbPtr;
        bulb.shader = lightShader;
        
        bulb.name = "pointLights";
        bulb.ambient = vec3(1);
        bulb.diffuse = vec3(1);
        bulb.specular = vec3(1);
        bulb.k0 = 1;
        bulb.k1 = 0.0005;
        bulb.k2 = 0.00004;

        std::shared_ptr<Bulb> ptr(bulbPtr);
        scene.addLight(ptr);
        scene.addModel(ptr);
#endif
    }
     
    return scene;
}

mat4 config::projectionMatrix(float fovDegree) {
    return perspective(radians(fovDegree), 1.f * screenPixelW / screenPixelH, 0.1f, cameraFarPlane);
}

Camera* config::loadCamera() {
    auto camera = new Camera();
    camera->position = vec3(0.f, 4.5f, 10.3f);
    camera->front = normalize(vec3(0.f, 0.f, -1.f));
    camera->upVec = vec3(0.f, 1.f, 0.f);
    camera->fov = 38;
    return camera;
}

#if NDEBUG
#else
std::map<int, size_t> EfficiencyTool::TIMES;
std::map<int, size_t> EfficiencyTool::TOTAL;
#endif
