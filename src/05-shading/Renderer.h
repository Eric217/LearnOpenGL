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
#include "Camera.h"

#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Scene {
    
};

class Renderer {
public:
    glm::mat4 modelM;
    glm::mat4 model2M;
    glm::mat4 viewM;
    glm::mat4 projectionM;
    
    Camera *camera;
public:
    Renderer();
    virtual ~Renderer();
    
    void render();
     
private:
    GLuint vbo;
    GLuint vao;
    GLuint ebo;
    Shader *shader;
    Shader *lampShader;
    std::vector<Texture *> textures;
};


#endif /* Renderer_hpp */
