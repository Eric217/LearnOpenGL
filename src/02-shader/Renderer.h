//
//  Renderer.hpp
//  01
//
//  Created by Eric on 2022/9/23.
//

#ifndef Renderer_hpp
#define Renderer_hpp

#include "Shader.h"
#include <glad/glad.h>

class Renderer {
public:
    Renderer();
    virtual ~Renderer();
    
    void render();
    
private:
    GLuint vbo;
    GLuint vao;
    GLuint ebo;
    Shader *shader;
};


#endif /* Renderer_hpp */
