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

int config::initWindowPointSizeW = 900;
int config::initWindowPointSizeH = 720;

int config::screenPixelW = 0;
int config::screenPixelH = 0;

#define useCubemap 0

Scene config::loadScene() {
    mat4 id4(1.f);
    std::string modelDir = MODEL_DIR;
    std::string shaderDir = SHADER_DIR;

    Scene scene;
 
    // skybox
    {
        auto boxShader = Shader(shaderDir + "/skybox/v.shader", shaderDir + "/skybox/f.shader");
        auto box = new Model(modelDir + "/skybox/skybox.obj", id4, GL_CLAMP_TO_EDGE, boxShader, false);
        
        std::vector<Texture> ts;
        for (int i = 0; i < 6; i++) {
            auto path = modelDir + "/skybox/" + std::to_string(i) + ".jpg";
            ts.push_back(Texture::load(path, DIFFUSE_TEXTURE, false));
            ts[i].shouldUse = !useCubemap;
        }
        CubeTexture cube(modelDir + "/skybox/");
        cube.shouldUse = useCubemap;
        ts.push_back(std::move(cube));
        box->setTextures(std::move(ts));
         
        scene.skybox.reset(box);
    }
    
    // cube
    {
        auto shader = Shader(shaderDir + "/cube/v.shader", shaderDir + "/cube/f.shader");
        scene.addModel(modelDir + "/cube/cube.obj",
                       scale(translate(id4, vec3(0.1,0.1,-0.2)), vec3(0.34)),
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
