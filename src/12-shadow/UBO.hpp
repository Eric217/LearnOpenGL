//
//  UBO.hpp
//  10-geometry
//
//  Created by Eric on 2022/10/30.
//

#ifndef UBO_hpp
#define UBO_hpp

#include <glad/glad.h>
#include <glm/glm.hpp>

// 一个 buffer 有一个 ID，包含大块数据，通过 subData 设置
// 一个 shader 可以有多个 block，一个 block 是一大块数据，一个 block 对应一个 buffer
// block 和 buffer 通过 OpenGL 槽位（binding point）联系在一起
// 把 shader 的 block id 绑定到一个槽位；然后把 buffer 绑到该槽位

struct UniformBuffer {
    GLuint ID;
    
    UniformBuffer() { glGenBuffers(1, &ID); }
    
    void bindRange(int start, int len, int slot) {
        glBindBufferRange(GL_UNIFORM_BUFFER, slot, ID, start, len);
    };
};

/// 管理 N 个 mat4 的
struct UniformBufferM4: UniformBuffer {
    UniformBufferM4(int count, GLenum usage);
    void update(int index, const glm::mat4 &m);
    /// 用 vec3 一次更新 16 字节
    void update(int index, const glm::vec3 &row);
};

/// 管理一个 vec3 的
struct UniformBuffer3f: UniformBuffer {
    UniformBuffer3f(GLenum usage);
    
    void update(const glm::vec3 &v);
    void update(int index, float value);
};

#endif /* UBO_hpp */
