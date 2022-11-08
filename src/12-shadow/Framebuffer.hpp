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

class Framebuffer {
protected:
    GLuint Id;
    GLuint texId; // for color，子类可能用途不一样
    GLuint rbo; // for depth and stencil，子类可能用途不一样
public:
    int w, h;
    
public:
    Framebuffer(int w, int h);
    
    virtual ~Framebuffer() {};
    
    GLuint asTexture() const { return texId; }
    
    void activate() const {
        glBindFramebuffer(GL_FRAMEBUFFER, Id);
        glViewport(0, 0, w, h);
    };
    void deactivate() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); };
    
    virtual void updateSize(int w, int h);
    
protected:
    Framebuffer() {};
    
};

class DepthFramebuffer: public Framebuffer {

public:
    DepthFramebuffer(int w, int h);
    void updateSize(int w, int h);
};

#endif /* Framebuffer_hpp */
