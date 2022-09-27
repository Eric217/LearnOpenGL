#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Renderer.h"
#include <unistd.h>

static void framebuffer_size_change(GLFWwindow *window, int w, int h);
static void processKey(GLFWwindow *window);

static Renderer *renderer;

int main() {
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    GLFWwindow *window = glfwCreateWindow(800, 600, "03", 0, 0);
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
   
    renderer = new Renderer();
   
    while (!glfwWindowShouldClose(window)) {
        processKey(window);
       
        renderer->render();
    
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    delete renderer;
    glfwTerminate();
    return 0;
}

static void framebuffer_size_change(GLFWwindow *window, int w, int h) {
    glViewport(0, 0, w, h);
}

// 这样处理键盘可能每按一次处理好几次，以后优化
static void processKey(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, 1);
    } else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        renderer->updateMixLevel(true);
    } else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        renderer->updateMixLevel(false);
    }
}
