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

#define GL_GLEXT_PROTOTYPES
#include <glext.h>

using namespace glm;
using namespace config;

#define UNIFORM_VP 0

#define INSTANCE_COUNT 1000

static GLuint instanceBuffer;

// instancing 渲染，每个实例有不同的缩放、位置、旋转，即不同的 model 矩阵
//
// 首先 model 需要添加一组顶点属性，并且设置为每个实例更新一次
// 传输该属性 bufferData；绑定顶点属性时只能一次绑一个 vec4，因此其实是 4 组
// shader 中可直接当作 mat4 对待，有 gl_InstanceID 获取实例序号
//

static auto arr = new mat4[INSTANCE_COUNT]();
static mat4 id4(1);
static float radius = 20;

void Renderer::setup(const Scene &scene) {
    EfficiencyTool tool(0, 0);
    srand(time(0));
    auto &rock = scene.modelAt(0).meshes[0];
 
    glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
    for (int i = 0; i < INSTANCE_COUNT; i++) {
        int r1 = rand() % 360;
        int r2 = rand() % 22;
        float arg1 = (rand() % 100 - 50) * 0.08f;
        float arg2 = (rand() % 100 - 50) * 0.08f;
        float theta = r1 * M_PI / 180;
        // 随便写的 transform
        arr[i] = rotate(scale(translate(id4,
            vec3(std::sin(theta) * radius + arg1, ((rand()) % 20 - 10) * 0.15f, std::cos(theta) * radius) + arg2),
            glm::vec3((r2 % 22) / 100.0f + 0.03)), // scale
            (float)r1, vec3(r1,arg1,arg2)); // rotate
    }
    glBufferData (GL_ARRAY_BUFFER, sizeof(mat4) * INSTANCE_COUNT, arr, GL_STREAM_DRAW);
}

void Renderer::render(Scene& scene, const Camera *camera) {
    vBuffer.update(0, camera->getViewMatrix());
    vBuffer.update(1, config::projectionMatrix(camera->fov));
    
    auto value = (duration_cast<std::chrono::milliseconds>
                  (std::chrono::system_clock::now().time_since_epoch())).count();
    render1(scene, camera);
}
 
void Renderer::render1(Scene& scene, const Camera *camera) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
    scene.modelAt(1).draw(true);
    
    auto &rock = scene.modelAt(0).meshes[0];
    glBindVertexArray(rock.vao);
    scene.modelAt(0).shader.use();
    glDrawElementsInstanced (GL_TRIANGLES, rock.indices.size(), GL_UNSIGNED_INT, 0, INSTANCE_COUNT);
    
    if (scene.skybox) {
        glFrontFace(GL_CW);
        glDepthFunc(GL_LEQUAL);
        scene.skybox.get()->draw(true);
        glDepthFunc(GL_LESS);
        glFrontFace(GL_CCW);
    }
    
}

void Renderer::updateScreenSize(int w, int h) {
    glViewport(0, 0, w, h);
}

Renderer::Renderer(const Scene &scene) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    // 手动定义槽位意义：绑定好相关的全局 uniform，ubo
    // 0 - view
    // 1 - projection
    // 2 - camera pos
    auto models = scene.allModels();
    for (auto &model: models) {
        if (!model) {
            continue;
        }
        model.get()->shader.use();
        model.get()->bindUniformBlock("ViewProjection", UNIFORM_VP);
    }
    vBuffer.bindRange(0, 2 * sizeof(mat4), UNIFORM_VP);
     
    auto &rock = scene.modelAt(0).meshes[0];
    
    glBindVertexArray(rock.vao);
    scene.modelAt(0).applyTextures();
    
    glGenBuffers(1, &instanceBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
    
    glBufferData (GL_ARRAY_BUFFER, sizeof(mat4) * INSTANCE_COUNT, 0, GL_STREAM_DRAW);
    glVertexAttribPointer (3, 4, GL_FLOAT, 0, sizeof(mat4), (void*)(0 * sizeof(vec4)));
    glVertexAttribPointer (4, 4, GL_FLOAT, 0, sizeof(mat4), (void*)(1 * sizeof(vec4)));
    glVertexAttribPointer (5, 4, GL_FLOAT, 0, sizeof(mat4), (void*)(2 * sizeof(vec4)));
    glVertexAttribPointer (6, 4, GL_FLOAT, 0, sizeof(mat4), (void*)(3 * sizeof(vec4)));
    
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
    
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(6, 1);
    glVertexAttribDivisor(5, 1);

    glBindVertexArray(0);
    setup(scene);
}
