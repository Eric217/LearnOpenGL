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
    GLuint Id;
    GLuint texId; // for color
    GLuint rbo; // for depth and stencil
public:
    int w, h;
    
public:
    Framebuffer(int w, int h);
    ~Framebuffer() { glDeleteFramebuffers(1, &Id); };
    
    GLuint asTexture() const { return texId; }
    
    void activate() const {
        glBindFramebuffer(GL_FRAMEBUFFER, Id);
        glViewport(0, 0, w, h);
    };
    void deactivate() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); };
    
    void updateSize(int w, int h);
};

#endif /* Framebuffer_hpp */
