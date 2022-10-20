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
#include "Model.hpp"
#include "Light.h"

#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Scene {
    friend class Renderer;
public:
    void addModel(const std::string& path, const glm::mat4 &trans, GLenum wrap = GL_REPEAT) {
        models.push_back(Model(path, trans, wrap));
    }
    
//    void addLight
private:
    std::vector<Model> models;
    std::vector<PointLight> lights;
};

class Renderer {
public:
    glm::mat4 model2M;
    glm::mat4 viewM;
    glm::mat4 projectionM;
    
    Camera *camera;
public:
    Renderer();
    virtual ~Renderer();
    
    void render(Scene& scene);
     
private:
    GLuint vbo;
    GLuint vao;
    GLuint ebo;
    
    Shader *shader;
    Shader *lampShader;
    Shader *borderShader;
    Shader *grassShader;
    
    std::vector<Texture *> textures;
    std::vector<Shader *> shaders;
};


#endif /* Renderer_hpp */
