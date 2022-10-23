//
//  Config.hpp
//  08-framebuffer
//
//  Created by Eric on 2022/10/22.
//

#ifndef Config_hpp
#define Config_hpp

#include <glm/glm.hpp>

class Scene;
class Camera;

namespace config {

extern int initWindowPointSizeW;
extern int initWindowPointSizeH;

extern int screenPixelW;
extern int screenPixelH;

Scene loadScene();

/// 调用者负责 delete
Camera* loadCamera();

glm::mat4 projectionMatrix(float fovDegree);


}

#endif /* Config_hpp */
