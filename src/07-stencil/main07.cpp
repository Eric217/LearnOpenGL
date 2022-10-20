#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Renderer.h"
#include "Camera.h"
#include <unistd.h>

static void framebuffer_size_change(GLFWwindow *window, int w, int h);
static void processKey(GLFWwindow *window);
static void processMouse(GLFWwindow*, double x, double y);
static void processScroll(GLFWwindow*, double xOffset, double yOffset);

using namespace glm;

constexpr mat4 identityM(1.f);

static int screenH = 400;
static int screenW = 600;
static double frameDuration = 0;

static mat4 getProjectionMatrix(float fovDegree) {
    return perspective(radians(fovDegree), 1.f * screenW / screenH, 0.1f, 100.f);
}

static Camera *camera;

int main() {
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    GLFWwindow *window = glfwCreateWindow(screenW, screenH, TARGET_NAME, 0, 0);
    if (!window) {
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return -1;
    }
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_change);
   
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, processMouse);
    
    glfwSetScrollCallback(window, processScroll);
    
    Renderer *renderer = new Renderer();
    camera = new Camera();
    camera->position = vec3(0.f, 4.5f, 10.3f);
    camera->front = normalize(vec3(0.f, 0.f, -1.f));
    camera->upVec = vec3(0.f, 1.f, 0.f);
    camera->fov = 35;
    renderer->camera = camera;
  

    Scene scene;
    std::string modelDir = MODEL_DIR;
    auto trans = mat4(1.f);
    auto trans1 = vec3(-2.4, 0.01, 0.5);
    const float borderW = 1.05;
    trans = translate(trans, trans1);
    scene.addModel(modelDir + "/cube/cube.obj", trans);
    // glm 的调用顺序是反的，先调用平移再缩放（矩阵乘法是 T * S * Mat)
    scene.addModel(modelDir + "/cube/cube.obj", scale(translate(mat4(1.f), trans1), vec3(borderW)));

    trans1 = vec3(2.0, 0.01, -0.4);
    trans = translate(mat4(1.f), trans1);
    scene.addModel(modelDir + "/cube/cube.obj", trans);
    scene.addModel(modelDir + "/cube/cube.obj", scale(translate(mat4(1), trans1), vec3(borderW)));
    
    scene.addModel(modelDir + "/floor/floor.obj", mat4(1.f));
    // 这个用 grass shader
    float _x = 0;
    scene.addModel(modelDir + "/quadr/quadr.obj", translate(mat4(1), vec3(_x, 0, 1.8)), GL_CLAMP_TO_EDGE);
    // 这几个窗户用普通 shader
    scene.addModel(modelDir + "/quadr/quadr.obj", translate(mat4(1), vec3(_x, 0, 0.1)), GL_CLAMP_TO_EDGE);
    scene.addModel(modelDir + "/quadr/quadr.obj", translate(mat4(1), vec3(_x, 0, -4.1)), GL_CLAMP_TO_EDGE);
    scene.addModel(modelDir + "/quadr/quadr.obj", translate(mat4(1), vec3(_x, 0, 3.53)), GL_CLAMP_TO_EDGE);

    // 灯的位置
    auto mat2 = translate(identityM, vec3(0.5f, 5.f, 1.7f));
    mat2 *= scale(identityM, vec3(0.3f, 0.3f, 0.3f));
    
    double lastTime = glfwGetTime();
    
    while (!glfwWindowShouldClose(window)) {
        processKey(window);
        glClearColor(0.1f, 0.1f, 0.1f, 1.f);
        
        // model 暂时写死在 renderer 中，有两个立方体盒子
        renderer->model2M = mat2;
        
        renderer->viewM = camera->getViewMatrix();
        
        renderer->projectionM = getProjectionMatrix(camera->fov);
        
        renderer->render(scene);
   
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
    glViewport(0, 0, w, h);
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
