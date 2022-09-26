#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Renderer.h"
#include <unistd.h>

static void framebuffer_size_change(GLFWwindow *window, int w, int h);
static void processKey(GLFWwindow *window);

int main() {
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    GLFWwindow *window = glfwCreateWindow(800, 600, "02", 0, 0);
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
   
    Renderer renderer;
   
    while (!glfwWindowShouldClose(window)) {
        processKey(window);
       
        renderer.render();
    
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}

static void framebuffer_size_change(GLFWwindow *window, int w, int h) {
    glViewport(0, 0, w, h);
}

static void processKey(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, 1);
    }
}
