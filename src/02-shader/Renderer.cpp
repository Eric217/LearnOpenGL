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

static GLfloat vertices[] = {
    // pos            | color
    -0.5f, -0.5f, 0.0f, 1, 0, 0,
    0.5f, -0.5f, 0.0f,  0, 1, 0,
    0.0f,  0.5f, 0.0f,  0, 0, 1
};

static GLuint indices[] = {
    0, 1, 2
};

static float zValue = 0;

void Renderer::render() {
    if (shader) {
        shader->use();
        zValue += M_PI / 60;
        shader->setFloat("customX", sin(zValue) / 2);
    }
  
    glClearColor(0.2f, 0.3f, 0.3f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBindVertexArray(vao);
    glDrawElements (GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
}

Renderer::Renderer() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);
   
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
     
    std::string dir = SHADER_DIR;
    shader = new Shader(dir + "/v.shader", dir + "/f.shader");
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
}

