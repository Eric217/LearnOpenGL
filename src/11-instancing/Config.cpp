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

float config::cameraStep = 15;

#define useCubemap 0

Scene config::loadScene() {
    const std::string modelDir = SKY_MODEL_DIR;
    const std::string planetDir = GLOBAL_MODEL_DIR"/planet/planet.obj";
    const std::string rockDir = GLOBAL_MODEL_DIR"/rock/rock.obj";
    const std::string shaderDir = SHADER_DIR;

    Scene scene;
    mat4 id4(1.f);

    std::vector<Texture> cubemapTextures;
    // skybox
    {
        auto boxShader = Shader(shaderDir + "/skybox/v.shader", shaderDir + "/skybox/f.shader");
        auto box = new Model(modelDir + "/skybox/skybox.obj", boxShader, id4, GL_CLAMP_TO_EDGE, false);

        for (int i = 0; i < 6; i++) {
            auto path = modelDir + "/skybox/" + std::to_string(i) + ".jpg";
            cubemapTextures.push_back(Texture::load(path, CUBEMAP2D_TEXTURE, false));
            cubemapTextures[i].shouldUse = !useCubemap;
        }
        CubeTexture cube(modelDir + "/skybox/");
        cube.shouldUse = useCubemap;
        cubemapTextures.push_back(std::move(cube));
        box->appendTextures(cubemapTextures);

        scene.skybox.reset(box);
    }
   
    // models
    {
        auto shader = Shader(shaderDir + "/rock/v.vs", shaderDir + "/rock/f.fs");
        auto m = new Model(rockDir, shader);
        scene.addModel(m);
    }
    {
        auto shader = Shader(shaderDir + "/planet/v.vs", shaderDir + "/planet/f.fs");
        scene.addModel(planetDir, scale(id4, vec3(1.45)), shader);
    }
    return scene;
}

mat4 config::projectionMatrix(float fovDegree) {
    return perspective(radians(fovDegree), 1.f * screenPixelW / screenPixelH, 0.1f, 200.f);
}

Camera* config::loadCamera() {
    auto camera = new Camera();
    camera->position = vec3(0.f, 4.5f, 30.3f);
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
