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
public:
    int w, h;
    
public:
    virtual ~Framebuffer() {};
     
    void activate() const {
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

class NormalFramebuffer: public Framebuffer {
    GLuint texId; // for color
    GLuint rbo; // for depth and stencil
public:
    NormalFramebuffer(int w, int h);
 
    GLuint asTexture() const { return texId; }
    void updateSize(int w, int h);

};

class DepthFramebuffer: public Framebuffer {
    GLuint texId; // for depth

public:
    DepthFramebuffer(int w, int h);
    
    void updateSize(int w, int h);
    GLuint asTexture() const { return texId; }
};

class CubeDepthFramebuffer: public Framebuffer {
    GLuint texId; // for depth

public:
    CubeDepthFramebuffer(int w, int h);
    
    void updateSize(int w, int h);
    GLuint asTexture() const { return texId; }
};


#endif /* Framebuffer_hpp */
