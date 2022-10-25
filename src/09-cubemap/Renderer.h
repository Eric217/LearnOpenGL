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


class Renderer {
    std::vector<Texture *> textures;
    glm::mat4 viewM;
    glm::mat4 projectionM;
    
public:
    Renderer();
    virtual ~Renderer();
    
    void render(Scene& scene, const Camera *camera);
    void updateScreenSize(int w, int h);
    
private:
    void applyViewProjectionMatrix(const Shader &shader) const;
    
    void render1(Scene& scene);
    
};


#endif /* Renderer_hpp */
