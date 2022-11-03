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
#include "UBO.hpp"

#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>


class Renderer {
    glm::mat4 viewM;
    glm::mat4 projectionM;
    
    UniformBufferM4 vBuffer;

public:
    Renderer(const Scene &scene);
    ~Renderer(){};
    
    void render(Scene& scene, const Camera *camera);
    void updateScreenSize(int w, int h);
    
private:
    void setup(const Scene &scene);
    void render1(Scene& scene, const Camera *camera);
    
};


#endif /* Renderer_hpp */
