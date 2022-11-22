//
//  Framebuffer.hpp
//  08-framebuffer
//
//  Created by Eric on 2022/10/23.
//

#ifndef Framebuffer_hpp
#define Framebuffer_hpp

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>

/// 虚基类
class Framebuffer {
public:
    int w, h;
    
public:
    virtual ~Framebuffer() {};
     
    virtual void activate() const {
        glBindFramebuffer(GL_FRAMEBUFFER, Id);
        glViewport(0, 0, w, h);
    };
    void deactivate() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); };
    
    virtual void updateSize(int w, int h) = 0;
    virtual GLuint asTexture() const = 0;
    
protected:
    GLuint Id;
protected:
    
};

/// 一个用于离屏渲染的普通 buffer，含颜色、深度、stencil
class NormalFramebuffer: public Framebuffer {
protected:
    GLuint texId; // for color0
    GLuint rbo; // for depth and stencil
    bool usingHdr;
public:
    NormalFramebuffer(int w, int h, bool hdr = false);
 
    GLuint asTexture() const { return texId; }
    void updateSize(int w, int h);

};

class MRTNormalBuffer: public NormalFramebuffer {
    std::vector<GLuint> texIds;
public:
    MRTNormalBuffer(int w, int h, int targetCount = 1, bool hdr = false);
    
    GLuint getTextureAt(int index) const { return texIds[index]; }
    
    void updateSize(int w, int h);
};

/// 只有 深度 的 2D 纹理
class DepthFramebuffer: public Framebuffer {
    GLuint texId; // for depth

public:
    DepthFramebuffer(int w, int h);
    
    void updateSize(int w, int h);
    GLuint asTexture() const { return texId; }
};

/// 只有 深度 的 cubemap 纹理
class CubeDepthFramebuffer: public Framebuffer {
    GLuint texId; // for depth

public:
    CubeDepthFramebuffer(int w, int h);
    
    void updateSize(int w, int h);
    GLuint asTexture() const { return texId; }
};

/// 用于后处理的乒乓 buffer，只有两个颜色 buffer
class PingPongBuffer: public Framebuffer {
protected:
    // GLuint Id;
    GLuint anotherId;
    
    GLuint usingBuffer;
    
    GLuint texId[2]; // for color

    bool usingHdr;

public:
    
    PingPongBuffer(int w, int h, bool usingHdr);
     
    void activate() const {
       glBindFramebuffer(GL_FRAMEBUFFER, usingBuffer);
       glViewport(0, 0, w, h);
    }
    void activate(int targetBufferIndex) {
        usingBuffer = targetBufferIndex == 0 ? Id : anotherId;
        activate();
    }
    bool usingPing() { return usingBuffer == Id; }
    void usePing() { activate(0); }
    void usePong() { activate(1); }
    GLuint pingTexture() const { return texId[0]; }
    GLuint pongTexture() const { return texId[1]; }

    void updateSize(int w, int h);
    
    GLuint asTexture() const { return texId[usingBuffer != Id]; }
     
};

#endif /* Framebuffer_hpp */
