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
#include "Framebuffer.hpp"
#include "Scene.hpp"

#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>


class Renderer {
    ContextUBO context;
    UniformBufferM4 vBuffer;
    /// 4 vec3 + mat4 view
    UniformBufferM4 dirLightBuffer;
    UniformBufferM4 pointLightBuffer;
    std::vector<UniformBufferM4> cubeMatrices;
    
    std::vector<DepthFramebuffer> dirShadowMaps;
    std::vector<CubeDepthFramebuffer> pointShadowMaps;

    std::shared_ptr<MRTNormalBuffer> hdrBuffer;
    std::shared_ptr<PingPongBuffer> ppBuffer;
    
    std::shared_ptr<MRTNormalBuffer> deferredBuffer;
    
    std::shared_ptr<MRTNormalBuffer> tmpBuffer;
    std::shared_ptr<MRTNormalBuffer> tmp2Buffer;

    std::shared_ptr<Model> tmpQuad;
    Shader imageDrawer; // out 1 个 color
    Shader image2Drawer; // out 2 个 color

public:
    Renderer(const Scene &scene);
    ~Renderer(){};
    
    void render(Scene& scene, const Camera *camera);
    void updateScreenSize(int w, int h);
    
private:
    void setup(const Scene &scene);
    void render1(Scene& scene, const Camera *camera, const Shader *customShader = 0);
    void render2(Scene& scene, const Camera *camera);
    void render3(Scene& scene, const Camera *camera);
    void renderHDR(Scene& scene, const Camera *camera, const MRTNormalBuffer &buffer);
    
    const MRTNormalBuffer& getHdrBuffer();
    const MRTNormalBuffer& getDeferredBuffer();
    void renderImage(GLuint id) const;
};


#endif /* Renderer_hpp */
