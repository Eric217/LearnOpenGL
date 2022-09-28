//
//  Renderer.cpp
//  01
//
//  Created by Eric on 2022/9/23.
//

#include "Renderer.h"
#include "Shader.h"

#include <glad/glad.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <algorithm>

static GLfloat vertices[] = {
    // 左上角顺时针 pos | texture coordinate
    -0.5f, 0.5f, 0.0f,  0, 1,
    0.5f, 0.5f, 0.0f,   1, 1,
    0.5f, -0.5f, 0.0f,  1, 0,
    -0.5f, -0.5f, 0.0f, 0, 0
};

static GLuint indices[] = {
    0, 1, 2,
    2, 0, 3
};

void Renderer::render() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBindVertexArray(vao);
    glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

Renderer::Renderer() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);
   
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
     
    std::string dir = SHADER_DIR;
    shader = new Shader(dir + "/v.shader", dir + "/f.shader");

    dir = TEXTURE_DIR;
    textures.push_back(new Texture(dir + "/container.jpg"));
    textures.push_back(new Texture(dir + "/awesomeface.png"));
    
    shader->use();
    // 给 shader 的纹理对象设置单元号，然后把纹理数据绑定到单元号
    for (auto i = 0; i < textures.size() && shader; i++) {
        shader->setInt("image" + std::to_string(i), i);
        textures[i]->use(i);
    }
    shader->setFloat("mixLevel", 0);
}

Renderer::~Renderer() {
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &ebo);
 
    if (shader) {
        delete shader;
    }
    for (Texture *t: textures) {
        if (t) {
            delete t;
        }
    }
}

void Renderer::updateMixLevel(bool isUp) {
    if (!shader) return;
    auto loc = glGetUniformLocation(shader->ID, "mixLevel");
    if (loc == -1) return;
    float mixLevel;
    glGetUniformfv(shader->ID, loc, &mixLevel);
    float newLevel = mixLevel + 0.1 * (isUp ? 1 : -1);
    shader->setFloat("mixLevel", std::min(1.f, std::max(0.f, newLevel)));
}