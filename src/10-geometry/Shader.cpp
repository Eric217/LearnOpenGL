//
//  Renderer.cpp
//  01
//
//  Created by Eric on 2022/9/23.
//

#include "Shader.h"
#include <glad/glad.h>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>

void Shader::use() const {
    glUseProgram(ID);
}

// 调用 glUniform 前面必须把 program 绑到状态机！
// uniform 必须有用到才能取到 location，否则出错！
// 关于后缀：3f = 3 个 float 参数；
// [1-4]fv，1-4 用来说明数据类型是 f 或 vec[2-4]，数组 count 是由参数 count 确定，
// 例如 uniform vec2 arr[5]，在 C 里 count 是 5，数据用 float data[10]

void Shader::setBool(const std::string &name, bool value) const {
    auto loc = glGetUniformLocation(ID, name.c_str());
    glUniform1i(loc, value);
}

void Shader::setInt(const std::string &name, int value) const {
    auto loc = glGetUniformLocation(ID, name.c_str());
    glUniform1i(loc, value);
}

void Shader::setVec3(const std::string &name, const glm::vec3& value) const {
    auto loc = glGetUniformLocation(ID, name.c_str());
    glUniform3fv(loc, 1, glm::value_ptr(value));
}

void Shader::setFloat(const std::string &name, float value) const {
    auto loc = glGetUniformLocation(ID, name.c_str());
    glUniform1f(loc, value);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &matrix) const {
    auto loc = glGetUniformLocation(ID, name.c_str());
    glUniformMatrix4fv(loc, 1, false, glm::value_ptr(matrix));
}

void Shader::setMat3(const std::string &name, const glm::mat4 &matrix) const {
    auto loc = glGetUniformLocation(ID, name.c_str());
    glUniformMatrix3fv(loc, 1, false, glm::value_ptr(glm::mat3(matrix)));
}

void Shader:: bindUniformBlock(const std::string &name, int slot) const {
    auto index = glGetUniformBlockIndex(ID, name.c_str());
    glUniformBlockBinding(ID, index, slot);
}

// MARK: - init

/// 返回 shader id，参数 not null
static GLuint makeShader(const char *path, GLenum type) {
    // 1. retrieve the source code from filePath
    std::string code;
    std::ifstream shaderFile;
    // ensure ifstream objects can throw exceptions:
    shaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        shaderFile.open(path);
        std::stringstream shaderStream;
        // read file's buffer contents into streams
        shaderStream << shaderFile.rdbuf();
        // close file handlers
        shaderFile.close();
        // convert stream into string
        code = shaderStream.str();
    }
    catch(std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        assert(false);
    }
    const char* shaderCode = code.c_str();
    int ok;
    char err[128] = {0};

    // 2. to gl
    auto s = glCreateShader(type);
    glShaderSource (s, 1, &shaderCode, 0);
    glCompileShader(s);
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        memset(err, 0, 128);
        glGetShaderInfoLog(s, 127, 0, err);
        std::cout << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n"
        << err << std::endl;
        assert(false);
    }
    return s;
}

static std::unordered_map<std::string, unsigned int> shaders;

/// 返回 program id，参数 vs fs not null，gs nullable
static GLuint makeProgram(const char *vs, const char *fs, const char *gs) {
    std::string key = std::string(vs) + "::" + fs;
    if (gs) {
        key.append(std::string("::") + gs);
    }
    auto found = shaders.find(key);
    if (found != shaders.end()) {
        return found->second;
    }
    
    GLuint ID = glCreateProgram();
    auto s1 = makeShader(vs, GL_VERTEX_SHADER);
    auto s2 = makeShader(fs, GL_FRAGMENT_SHADER);
    glAttachShader(ID, s1);
    glAttachShader(ID, s2);
    GLuint s3 = 0;
    if (gs) {
        s3 = makeShader(gs, GL_GEOMETRY_SHADER);
        glAttachShader(ID, s3);
    }
    glLinkProgram(ID);
    
    int ok;
    glGetProgramiv(ID, GL_LINK_STATUS, &ok);
    if (!ok) {
        char *err = (char *)calloc(512, 1);
        glGetProgramInfoLog(ID, 511, 0, err);
        std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n"
        << err << std::endl;
        assert(false);
        free(err);
    }
    
    glDeleteShader(s1);
    glDeleteShader(s2);
    if (s3) {
        glDeleteShader(s3);
    }
    if (ID) {
        shaders.emplace(key, ID);
    }
    return ID;
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath,
       const std::string& geometryPath) {
    ID = makeProgram(vertexPath.c_str(), fragmentPath.c_str(),
                     geometryPath.empty() ? 0 : geometryPath.c_str());
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    ID = makeProgram(vertexPath.c_str(), fragmentPath.c_str(), 0);
}
