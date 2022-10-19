//
//  Camera.h
//  01
//
//  Created by Eric on 2022/9/23.
//

#ifndef Camera_h
#define Camera_h

#include <string>
#include <glm/glm.hpp>

/// FPS 风格的相机
class Camera {
public:
    /// 相机位置
    glm::vec3 position;
    /// 看的方向
    glm::vec3 front;
    /// 指向 0 1 0 的向量
    glm::vec3 upVec;

    /// fov
    float fov;
    
    bool moved = true;
    
public:
    typedef enum MoveInstruction {
        LEFT, RIGHT, FORWARD, BACKWARD
    } _MoveInstruction;
    
    glm::mat4 getViewMatrix() const;
    /// 根据当前相机状态以及给定指令返回一个目标方向单位向量
    glm::vec3 move(MoveInstruction instruction) const;
    void zoom(float degreeDiff);
    void rotate(float pitchDiff, float yawDiff);

private:
    float pitch = 0, yaw = 0;
};


#endif /* Renderer_hpp */
