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

#define useCubemap 0

Scene config::loadScene() {
    const std::string modelDir = MODEL_DIR;
    const std::string nanosuitDir = GLOBAL_MODEL_DIR"/nanosuit/nanosuit.obj";
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
    auto none = std::string();
    auto triangle = shaderDir + "/triangle.gs";
    auto explode = shaderDir + "/explode.gs";
    auto normal = shaderDir + "/normal.gs";
    std::string *arr[4] = {&none, &triangle, &explode, &normal};
    
    // models
    {
        auto shader = Shader(shaderDir + "/nano/v.vs", shaderDir + "/nano/f.fs",
                             *arr[GEOMETRY_SHADER]);
        auto m = new Model(nanosuitDir, shader,
                           scale(translate(id4, vec3(0.1,1.1,0)), vec3(0.64)));
        m->appendTextures(cubemapTextures);
        scene.addModel(m);
    }
    {
        auto shader = Shader(shaderDir + "/cube/v.vs", shaderDir + "/cube/f.fs",
                             *arr[GEOMETRY_SHADER]);
        scene.addModel(modelDir + "/cube/cube.obj",
                       scale(translate(id4, vec3(0.2,9.1,2.7)), vec3(0.64)),
                       shader);
    }
    return scene;
}

mat4 config::projectionMatrix(float fovDegree) {
    return perspective(radians(fovDegree), 1.f * screenPixelW / screenPixelH, 0.1f, 100.f);
}

Camera* config::loadCamera() {
    auto camera = new Camera();
    camera->position = vec3(0.f, 4.5f, 10.3f);
    camera->front = normalize(vec3(0.f, 0.f, -1.f));
    camera->upVec = vec3(0.f, 1.f, 0.f);
    camera->fov = 38;
    return camera;
}
