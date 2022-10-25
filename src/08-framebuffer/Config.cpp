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


Scene config::loadScene() {
    mat4 identityM(1.f);
    std::string modelDir = MODEL_DIR;
    std::string shaderDir = SHADER_DIR;

    Scene scene;

    auto lightShader = Shader(shaderDir + "/v.shader", shaderDir + "/f_light.shader");
    auto defaultShader = Shader(shaderDir + "/v.shader", shaderDir + "/f.shader");
    auto borderShader = Shader(shaderDir + "/v.shader", shaderDir + "/f_border.shader");
    auto grassShader = Shader(shaderDir + "/v.shader", shaderDir + "/f_grass.shader");
    auto mirrorShader = Shader(shaderDir + "/v_rear_m.shader", shaderDir + "/f_rear_m.shader");
  
    auto trans = mat4(1.f);
    auto trans1 = vec3(-2.4, 0.01, 0.5);
    const float borderW = 1.05;
    trans = translate(trans, trans1);
    // 0/2、1/3 是箱子
    scene.addModel(modelDir + "/cube/cube.obj", trans, defaultShader);
    // glm 的调用顺序是反的，先调用平移再缩放（矩阵乘法是 T * S * Mat)
    scene.addModel(modelDir + "/cube/cube.obj", scale(translate(mat4(1.f), trans1), vec3(borderW)), borderShader);

    trans1 = vec3(2.0, 0.01, -0.4);
    trans = translate(mat4(1.f), trans1);
    scene.addModel(modelDir + "/cube/cube.obj", trans, defaultShader);
    scene.addModel(modelDir + "/cube/cube.obj", scale(translate(mat4(1), trans1), vec3(borderW)), borderShader);
    // 4 地板
    scene.addModel(modelDir + "/floor/floor.obj", mat4(1.f), defaultShader);
    
    // 5 灯 & 位置
    auto mat2 = translate(identityM, vec3(0.5f, 5.f, 1.7f));
    mat2 *= scale(identityM, vec3(0.3f, 0.3f, 0.3f));

    Bulb *bulbPtr = new Bulb(modelDir + "/cube/cube.obj", mat2);
    auto &bulb = *bulbPtr;
    bulb.shader = lightShader;
    
    bulb.name = "pointLights";
    bulb.ambient = vec3(1);
    bulb.diffuse = vec3(1);
    bulb.specular = vec3(1);
    bulb.k0 = 1;
    bulb.k1 = 0.0002;
    bulb.k2 = 0.000002;

    std::shared_ptr<Bulb> ptr(bulbPtr);
    scene.addLight(ptr);
    scene.addModel(ptr);
    
    // 6~9 草
    float _x = 0;
    scene.addModel(modelDir + "/quadr/quadr.obj", translate(mat4(1), vec3(_x, 0, 1.8)), grassShader, GL_CLAMP_TO_EDGE);
    // 这几个窗户用普通 shader
    scene.addModel(modelDir + "/quadr/quadr.obj", translate(mat4(1), vec3(_x, 0, 0.1)), grassShader, GL_CLAMP_TO_EDGE);
    scene.addModel(modelDir + "/quadr/quadr.obj", translate(mat4(1), vec3(_x, 0, -4.1)), grassShader, GL_CLAMP_TO_EDGE);
    scene.addModel(modelDir + "/quadr/quadr.obj", translate(mat4(1), vec3(_x, 0, 3.53)), grassShader, GL_CLAMP_TO_EDGE);
   
    float w_p = 0.2;
    // 三角形 x 从 -1～1 搞到 -0.2～0.2
    // 三角形 y 从 -1～1 搞到 0.6~1
    //       z 固定 -1（model 内可能不是 -1，下面设置 -1 可能没用，在 shader 中设
    auto t = translate(mat4(1), vec3(0, 1 - w_p, 0))
        * scale(mat4(1), vec3(w_p, w_p, -1));
    scene.rearMirror = std::shared_ptr<Model>(new Model(
        modelDir + "/quadr/quadr.obj", t, GL_CLAMP_TO_EDGE, mirrorShader));
    
    return (scene);
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
