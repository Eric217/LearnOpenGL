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

void Shader::use() {
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

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        vShaderFile.open(vertexPath.c_str());
        fShaderFile.open(fragmentPath.c_str());
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode   = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch(std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        assert(false);
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    int ok;
    char err[128] = {0};

    // 2. to gl
    auto vs = glCreateShader(GL_VERTEX_SHADER);
    auto fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource (vs, 1, &vShaderCode, 0);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        memset(err, 0, 128);
        glGetShaderInfoLog(vs, 127, 0, err);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
        << err << std::endl;
        assert(false);
    }
    
    glShaderSource (fs, 1, &fShaderCode, 0);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        memset(err, 0, 128);
        glGetShaderInfoLog(fs, 127, 0, err);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
        << err << std::endl;
        assert(false);
    }
    
    ID = glCreateProgram();
    glAttachShader(ID, vs);
    glAttachShader(ID, fs);
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &ok);
    if (!ok) {
        memset(err, 0, 128);
        glGetProgramInfoLog(ID, 127, 0, err);
        std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n"
        << err << std::endl;
        assert(false);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
}
