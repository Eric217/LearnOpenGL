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
    void update(int index, const glm::vec4 &row);

    template<typename T>
    void updateWord(int index, T value) {
        glBindBuffer(GL_UNIFORM_BUFFER, ID);
        glBufferSubData (GL_UNIFORM_BUFFER, index * 4, 4, &value);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
    
};

/// 管理一个 vec3 的
struct UniformBuffer3f: UniformBuffer {
    UniformBuffer3f(GLenum usage);
    
    void update(const glm::vec3 &v);
    void update(int index, float value);
};


/**
 保存一些上下文，和 shader 中结构对应，具体结构：
    type    name        start   len
    int     screen_w    0       4
    int     screen_h    4       4
    float   exposure    8       4
    float   farPlane_ps 12      4 // for point shadow
 */
class ContextUBO {
public:
    UniformBufferM4 ubo;

public:
    ContextUBO(int bindingPoint): ubo(1, GL_STATIC_DRAW) {
        ubo.bindRange(0, 1 * sizeof(glm::mat4), bindingPoint);
    };
    
    void updateScreenSize(int w, int h) {
        ubo.updateWord(0, w);
        ubo.updateWord(1, h);
    }
    void updateCameraExposure(float exposure) {
        ubo.updateWord(2, exposure);
    }
    void updateFarPlane_ps(float farPlane) {
        ubo.updateWord(3, farPlane);
    }
};

#endif /* UBO_hpp */
