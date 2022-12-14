//
//  Framebuffer.cpp
//  08-framebuffer
//
//  Created by Eric on 2022/10/23.
//

#include "Framebuffer.hpp"
#include <iostream>

// MARK: - normal buffer, like default
void NormalFramebuffer::updateSize(int w, int h) {
    this->w = w;
    this->h = h;
    glBindFramebuffer(GL_FRAMEBUFFER, Id);

    glBindTexture(GL_TEXTURE_2D, texId);
    GLenum fmt = usingHdr ? GL_RGB32F : GL_RGB;
    GLenum srcFmt = usingHdr ? GL_FLOAT : GL_UNSIGNED_BYTE;
    glTexImage2D (GL_TEXTURE_2D, 0/*level*/, fmt/*internalformat*/, w, h, 0/*border*/, GL_RGB /*data format*/ , srcFmt/*data type*/, 0/*data*/);
    
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_FRAMEBUFFER, GL_DEPTH24_STENCIL8, w, h);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

NormalFramebuffer::NormalFramebuffer(int w, int h, bool hdr) {
    this->w = w;
    this->h = h;
    usingHdr = hdr;
    
    glGenFramebuffers(1, &Id);
    // 可以指定读/写缓冲命令作用到不同的缓冲，一般使用同一个 GL_FRAMEBUFFER
    // GL_READ_FRAMEBUFFER | GL_DRAW_FRAMEBUFFER
    glBindFramebuffer(GL_FRAMEBUFFER, Id);
    
    // MARK: - 给 framebuffer attach buffers
    
    // 1. color buffer 用 texture 实现（可读，用于采样）
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    GLenum fmt = usingHdr ? GL_RGB32F : GL_RGB;
    GLenum srcFmt = usingHdr ? GL_FLOAT : GL_UNSIGNED_BYTE;
    glTexImage2D (GL_TEXTURE_2D, 0/*level*/, fmt/*internalformat*/, w, h, 0/*border*/, GL_RGB /*data format*/ , srcFmt/*data type*/, 0/*data*/);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // attach 到 fb，参数：target、attachment type、texture type、tex、level
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
    // 如果是用纹理作为深度/stencil buffer:
    // GL_DEPTH_COMPONENT(formats) + GL_DEPTH_ATTACHMENT(a type)
    // GL_STENCIL_INDEX(formats) + GL_STENCIL_ATTACHMENT(a type)
    // GL_DEPTH24_STENCIL8(internalformat) + GL_DEPTH_STENCIL(format)
    //   + GL_UNSIGNED_INT_24_8(data type) + GL_DEPTH_STENCIL_ATTACHMENT(a type)
    
    // 2. render buffer object as depth_stencil buffer
    // 特点：不采样、有优化
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);

    // attach
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    
    // check
    auto checkResult = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (checkResult != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER::INCOMPLETE::" << checkResult << std::endl;
        assert(false);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

MRTNormalBuffer::MRTNormalBuffer(int w, int h, int targetCount, bool hdr): NormalFramebuffer(w, h, hdr) {
    texIds.push_back(texId);
    if (targetCount <= 1) {
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, Id);

    for (int i = 1; i < targetCount; i++) {
        texIds.push_back(0);
        glGenTextures(1, &texIds[i]);
        glBindTexture(GL_TEXTURE_2D, texIds[i]);
        GLenum fmt = usingHdr ? GL_RGB32F : GL_RGB;
        GLenum srcFmt = usingHdr ? GL_FLOAT : GL_UNSIGNED_BYTE;
        glTexImage2D (GL_TEXTURE_2D, 0/*level*/, fmt/*internalformat*/, w, h, 0/*border*/, GL_RGB /*data format*/ , srcFmt/*data type*/, 0/*data*/);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texIds[i], 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MRTNormalBuffer::updateSize(int w, int h) {
    NormalFramebuffer::updateSize(w, h);
    glBindFramebuffer(GL_FRAMEBUFFER, Id);

    for (int i = 1; i < texIds.size(); i++) {
        glBindTexture(GL_TEXTURE_2D, texIds[i]);
        GLenum fmt = usingHdr ? GL_RGB32F : GL_RGB;
        GLenum srcFmt = usingHdr ? GL_FLOAT : GL_UNSIGNED_BYTE;
        glTexImage2D (GL_TEXTURE_2D, 0/*level*/, fmt/*internalformat*/, w, h, 0/*border*/, GL_RGB /*data format*/ , srcFmt/*data type*/, 0/*data*/);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// MARK: - for depth use only

DepthFramebuffer::DepthFramebuffer(int w, int h) {
    this->w = w;
    this->h = h;
    
    glGenFramebuffers(1, &Id);
    glBindFramebuffer(GL_FRAMEBUFFER, Id);
    glReadBuffer(GL_NONE);
    glDrawBuffer(GL_NONE);
    
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // frustum 过小时，外面的统一设置为亮的，border 取 1
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    float color[] = {1, 1, 1, 1};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
    
    // attach 到 fb，参数：target、attachment type、texture type、tex、level
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texId, 0);
    // check
    auto checkResult = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (checkResult != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER::INCOMPLETE::" << checkResult << std::endl;
        assert(false);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DepthFramebuffer::updateSize(int w, int h) {
    this->w = w;
    this->h = h;
    glBindFramebuffer(GL_FRAMEBUFFER, Id);
    assert(false);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// MARK: - for cube depth use only

CubeDepthFramebuffer::CubeDepthFramebuffer(int w, int h) {
    this->w = w;
    this->h = h;
    
    glGenFramebuffers(1, &Id);
    glBindFramebuffer(GL_FRAMEBUFFER, Id);
    glReadBuffer(GL_NONE);
    glDrawBuffer(GL_NONE);
    
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texId);
    for (int i = 0; i < 6; i++) {
        glTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    }
   
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    
    // attach 到 fb，参数：target、attachment type、tex、level
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texId, 0);
    // check
    auto checkResult = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (checkResult != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER::INCOMPLETE::" << checkResult << std::endl;
        assert(false);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CubeDepthFramebuffer::updateSize(int w, int h) {
    this->w = w;
    this->h = h;
    glBindFramebuffer(GL_FRAMEBUFFER, Id);
    assert(false);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texId);
    for (int i = 0; i < 6; i++) {
        glTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// MARK: - ping pong
PingPongBuffer::PingPongBuffer(int w, int h, bool usingHdr) {
    this->w = w;
    this->h = h;
    this->usingHdr = usingHdr;
    
    GLuint *buffers[2] = {&Id, &anotherId};
    glGenFramebuffers(1, buffers[0]);
    glGenFramebuffers(1, buffers[1]);
    glGenTextures(2, texId);
 
    for (int i = 0; i < 2; i++) {
        auto bufferId = *buffers[i];
        glBindFramebuffer(GL_FRAMEBUFFER, bufferId);
        glBindTexture(GL_TEXTURE_2D, texId[i]);
        
        GLenum fmt = usingHdr ? GL_RGB32F : GL_RGB;
        GLenum srcFmt = usingHdr ? GL_FLOAT : GL_UNSIGNED_BYTE;
        glTexImage2D (GL_TEXTURE_2D, 0/*level*/, fmt/*internalformat*/, w, h, 0/*border*/, GL_RGB /*data format*/ , srcFmt/*data type*/, 0/*data*/);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId[i], 0);
        // check
        auto checkResult = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (checkResult != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "ERROR::FRAMEBUFFER::INCOMPLETE::" << checkResult << std::endl;
            assert(false);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PingPongBuffer::updateSize(int w, int h) {
    for (int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, texId[i]);
        GLenum fmt = usingHdr ? GL_RGB32F : GL_RGB;
        GLenum srcFmt = usingHdr ? GL_FLOAT : GL_UNSIGNED_BYTE;
        glTexImage2D (GL_TEXTURE_2D, 0/*level*/, fmt/*internalformat*/,
                      w, h, 0/*border*/, GL_RGB /*data format*/ ,
                      srcFmt/*data type*/, 0/*data*/);
    }
}
