//
//  Framebuffer.cpp
//  08-framebuffer
//
//  Created by Eric on 2022/10/23.
//

#include "Framebuffer.hpp"
#include <iostream>

void Framebuffer::updateSize(int w, int h) {
    this->w = w;
    this->h = h;
    glBindFramebuffer(GL_FRAMEBUFFER, Id);

    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_FRAMEBUFFER, GL_DEPTH24_STENCIL8, w, h);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::Framebuffer(int w, int h): w(w), h(h) {
        
    glGenFramebuffers(1, &Id);
    // 可以指定读/写缓冲命令作用到不同的缓冲，一般使用同一个 GL_FRAMEBUFFER
    // GL_READ_FRAMEBUFFER | GL_DRAW_FRAMEBUFFER
    glBindFramebuffer(GL_FRAMEBUFFER, Id);
    
    // MARK: - 给 framebuffer attach buffers
    
    // 1. color buffer 用 texture 实现（可读，用于采样）
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D (GL_TEXTURE_2D, 0/*level*/, GL_RGB /*internalformat*/, w, h, 0/*border*/, GL_RGB /*data format*/ , GL_UNSIGNED_BYTE/*data type*/, 0/*data*/);
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
    
    // MARK: - check
    auto checkResult = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (checkResult != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER::INCOMPLETE::" << checkResult << std::endl;
        assert(false);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// MARK: - for depth use only

DepthFramebuffer::DepthFramebuffer(int w, int h): Framebuffer() {
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
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    
    // attach 到 fb，参数：target、attachment type、texture type、tex、level
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texId, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DepthFramebuffer::updateSize(int w, int h) {
    this->w = w;
    this->h = h;
    glBindFramebuffer(GL_FRAMEBUFFER, Id);
    assert(false);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    // MARK: - check
    auto checkResult = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (checkResult != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER::INCOMPLETE::" << checkResult << std::endl;
        assert(false);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
