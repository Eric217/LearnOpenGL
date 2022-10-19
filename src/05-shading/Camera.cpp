//
//  Camera.cpp
//  01
//
//  Created by Eric on 2022/9/23.
//

#include "Camera.h"
#include <glad/glad.h>
#include <assert.h>
#include <glm/gtc/type_ptr.hpp>
  
using namespace glm;

glm::vec3 Camera::move(MoveInstruction instruction) const {
    switch (instruction) {
        case FORWARD:
            return front;
        case BACKWARD:
            return -front;
        case RIGHT:
            return cross(front, upVec);
        case LEFT:
            return cross(upVec, front);
    }
}

void Camera::rotate(float pitchDiff, float yawDiff) {
    front.y = std::sin(pitchDiff);
    front.x = std::cos(pitchDiff) * std::cos(yawDiff);
    front.z = std::cos(pitchDiff) * std::sin(yawDiff);
    front = normalize(front);
}

void Camera::zoom(float degreeDiff) {
    fov = std::min(179.f, std::max(2.f, (fov + degreeDiff)));
}

static mat4 myLookAt(const vec3 &pos, const vec3 &target, const vec3 &up) {
    /// camera 看向 -z，因此 view space coordinate 的 z 轴实际是 camera 看的反方向！
    auto D = pos - target;
    auto R = normalize(cross(up, D));
    auto U = cross(D, R);
    mat4 id(1.f);
    id = translate(id, -pos);
    mat3 r(R, U, D);
    r = transpose(r);
    return mat4(r) * id;
}

glm::mat4 Camera::getViewMatrix() const {
#if !USING_GLM_LOOK_AT
    #define lookAt myLookAt
#endif
    return lookAt(position, position + front, upVec);
}
