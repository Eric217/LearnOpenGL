#include "Renderer.h"
#include "Camera.h"
#include "Config.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <unistd.h>

static void framebuffer_size_change(GLFWwindow *window, int w, int h);
static void processKey(GLFWwindow *window);
static void processMouse(GLFWwindow*, double x, double y);
static void processScroll(GLFWwindow*, double xOffset, double yOffset);

using namespace glm;

static double frameDuration = 0;
static Camera *camera;
static Renderer *renderer;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    GLFWwindow *window = glfwCreateWindow(config::initWindowPointSizeW, config::initWindowPointSizeH, TARGET_NAME, 0, 0);
    if (!window) {
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return -1;
    }
    glfwGetFramebufferSize(window, &config::screenPixelW, &config::screenPixelH);
    glViewport(0, 0, config::screenPixelW, config::screenPixelH);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_change);
   
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, processMouse);
    
    glfwSetScrollCallback(window, processScroll);
    
    camera = config::loadCamera();
    Scene scene = config::loadScene();
    renderer = new Renderer(scene);

    double lastTime = glfwGetTime();
    
    while (!glfwWindowShouldClose(window)) {
        processKey(window);
         
        renderer->render(scene, camera);
   
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        auto frameTime = glfwGetTime();
        frameDuration = frameTime - lastTime;
        lastTime = frameTime;
        usleep(12 * 1000); // CPU 占用太高了
    }
    delete renderer;
    delete camera;
    
    glfwTerminate();
    return 0;
}

static void framebuffer_size_change(GLFWwindow *window, int w, int h) {
    config::screenPixelW = w;
    config::screenPixelH = h;
    renderer->updateScreenSize(w, h);
}

/// fps 方式移动
static vec3 fpsMove(const vec3 &direction) {
    if (!direction.y) {
        return direction;
    }
    auto distance = direction;
    distance.y = 0;
    return normalize(distance);
}

static vec3 freeMove(const vec3 &dir) {
    return dir;
}

#define FPS_STYLE 0

/// 期望在 o 往 d 方向移动，处理碰撞检测等业务逻辑后返回实际移动向量
static vec3 resolve(const vec3 &origin, const vec3 &direction) {
    float delta = frameDuration * 4;
#if FPS_STYLE
    return fpsMove(direction) * delta;
#else
    return freeMove(direction) * delta;
#endif
}

static void processKey(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, 1);
        return;
    }
    vec3 moveDir(0.f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        moveDir += camera->move(Camera::FORWARD);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        moveDir += camera->move(Camera::BACKWARD);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        moveDir += camera->move(Camera::LEFT);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        moveDir += camera->move(Camera::RIGHT);
    }
    constexpr static vec3 zero(0.f);
    if (moveDir != zero) {
        camera->position += resolve(camera->position, normalize(moveDir));
        camera->moved = true;
    }
}

/// 这里左上角是原点，所以 y 取负
/// x y 起步为 0，但是我们刚开始要看向 -z，给 x 一个初始值
static void processMouse(GLFWwindow*, double x, double y) {
    static double mouseSpeed = 0.003;
    double pitch = -mouseSpeed * y;
    double yaw = mouseSpeed * x;
    camera->rotate(pitch, yaw - M_PI_2);
}

/// offset 是相对值；y 负代表缩小
static void processScroll(GLFWwindow*, double xOffset, double yOffset) {
    camera->zoom(-3 * yOffset);
}
