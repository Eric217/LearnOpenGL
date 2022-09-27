//
//  Renderer.hpp
//  01
//
//  Created by Eric on 2022/9/23.
//

#ifndef Renderer_hpp
#define Renderer_hpp

#include "Shader.h"
#include "Texture.h"
#include <glad/glad.h>
#include <vector>

class Renderer {
public:
    Renderer();
    virtual ~Renderer();
    
    void render();
    
    void updateMixLevel(bool isUp);
    
private:
    GLuint vbo;
    GLuint vao;
    GLuint ebo;
    Shader *shader;
    std::vector<Texture *> textures;
};


#endif /* Renderer_hpp */
