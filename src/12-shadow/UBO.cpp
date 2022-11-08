//
//  UBO.cpp
//  10-geometry
//
//  Created by Eric on 2022/10/30.
//

#include "UBO.hpp"
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
  
// MARK: - 2 mat4
UniformBufferM4::UniformBufferM4(int count, GLenum usage) {
    glBindBuffer(GL_UNIFORM_BUFFER, ID);
    glBufferData (GL_UNIFORM_BUFFER, count * sizeof(glm::mat4), 0, usage);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBufferM4::update(int index, const glm::mat4 &m) {
    glBindBuffer(GL_UNIFORM_BUFFER, ID);
    glBufferSubData (GL_UNIFORM_BUFFER, index * sizeof(mat4), sizeof(mat4), value_ptr(m));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBufferM4::update(int index, const glm::vec3 &row) {
    glBindBuffer(GL_UNIFORM_BUFFER, ID);
    glBufferSubData (GL_UNIFORM_BUFFER, index * sizeof(vec4), sizeof(vec3), value_ptr(row));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

 
// MARK: - 1 vec3

UniformBuffer3f::UniformBuffer3f(GLenum usage) {
    glBindBuffer(GL_UNIFORM_BUFFER, ID);
    glBufferData (GL_UNIFORM_BUFFER, sizeof(glm::vec3), 0, usage);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer3f::update(const glm::vec3 &v) {
    glBindBuffer(GL_UNIFORM_BUFFER, ID);
    glBufferSubData (GL_UNIFORM_BUFFER, 0, sizeof(vec3), value_ptr(v));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer3f::update(int index, float value) {
    glBindBuffer(GL_UNIFORM_BUFFER, ID);
    glBufferSubData (GL_UNIFORM_BUFFER, index * sizeof(float), sizeof(float), &value);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
