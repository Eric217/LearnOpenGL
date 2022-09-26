//
//  Renderer.cpp
//  01
//
//  Created by Eric on 2022/9/23.
//

#include "Renderer.h"
#include <glad/glad.h>
#include <assert.h>
#include <string.h>

#define LF "\n"

// 关于 shader

// 1.除基础数据类型还有 vector、matrix。(int, float, double, uint, bool)
// vecn 的 n 可以是 2-4，默认 float 类型，bvecn, ivecn, uvecn, dvecn
// 取四个数可以用 x y z w 或 r g b a 或 s t p q
// 可以快速构造另一个 vector 通过：someVec.xyzx（swizzling），因此最多 4 个元素 .xxxx
// 可以把 vec 作为构造函数的参数 vec4(someVec.xx, 1, 1)
// 2.shader 之间如果要“连接”起来，输出和输入的变量同类型、同名即可
// 3.全局变量用 uniform 修饰，先取位置再写入值，demo 和更多细节在下一节

// 可以不写 layout，用 glGetAttribLocation 得到 index 然后 enable；
static const char *vertext_shader =
"   #version 330 core"LF
"   layout (location = 0) in vec3 pos;"LF
"   void main() {"LF
"      gl_Position = vec4(pos.x, pos.y, pos.z, 1);"LF
"   }"LF
;

static const char *fragment_shader =
"   #version 330 core"LF
"   out vec4 f_color;"LF
"   void main() {"LF
"      f_color = vec4(1.0f, 0.5f, 0.2f, 1.0f);"LF
"   }"LF
;

// 初学问题：为什么这里用 NDC？那么什么时候执行的 viewport transform？
static float vertices[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f,  0.5f, 0.0f
};

void log(const char *fmt, ...);

static GLuint use_simple_shader();

void Renderer::renderVBO() {
    
    // 理解 OpenGL Buffer 对象操作流程：
    // 1 在 GPU 声明一块内存（创建 buffer object）
    // 2 说明这个对象是干啥用①的（buffer 类型），并【绑定到状态机】，后续对①类目标的读写就是用这个对象
    // 3 从 CPU 传输数据到 GPU，并说明将来怎么使用
    // 单次传输越多 效率越高
    
    GLuint vertexBufferObject;
    // 1
    glGenBuffers(1, &vertexBufferObject);
    // 2 指定为 array buffer，一般就是各个点各个坐标的数组
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    // 3 传输数据，说明使用方式
    // void (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
    // 使用方式：
    // GL_STREAM_DRAW 数据不会变、使用次数少；GL_STATIC_DRAW 不变、使用多；GL_DYNAMIC_DRAW 数据会变
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
     
    
    // 连接 vertex data(vbo) 与 shader vertex attributes
    // 给 OpenGL 解释当前状态机里的 vbo 是如何组织的、格式是什么
    // 背景：vertex data 是 vertex 的集合，vertex 是 attributes 的集合
    // 每个 attribute 分配一个序号。每个 attribute 由 1-4 个数据组成。
    // 硬件必须至少支持 16 个 attribute
    
    // void (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer)
    // 开始解释【一个 attribute】，参数依次是其：
    // 序号 用于 layout (location = ？)，
    // 元素数，1-4
    // 元素类型，
    // 是否对定点数 normalize，
    // 到下一个同类属性之间有多少字节空间（相当于所有属性所占空间）（紧密排列的可以传 0）
    // 第一个属性的第一个数据在 buffer 里的 offset
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    
    // enable attr at a location
    glEnableVertexAttribArray(0);
    
    // draw vbo：结合下面的 shader，调用 draw API。但在 Core 模式必须用 vao 才能画。
    // 以上用 vbo 时每次切换对象要重新输入数据、配置属性等，
    // 使用 vao，可以只配置一次，保存到 vao，后面切换对象只需要切换 vao
    // vao 具体会保存：attribute pointer 及其关联的 vbo，enable/disable
    
    use_simple_shader();
    
    // draw vbo and delete others
}

static float vertices_6[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f, 0.5f, 0.0f,
    0.6f, 0.6f, 0.0f,
    0.9f, 0.6f, 0.0f,
    0.75f, 0.9f, 0.0f
};

void Renderer::renderVAO() {
    // vao 具体会保存：attribute pointer 及其关联的 vbo，enable/disable
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    GLuint vertexBufferObject;
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_6), vertices_6, GL_STATIC_DRAW);
    // 配置会保存到 vao
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);

    auto shader = use_simple_shader();
    
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteProgram(shader);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vertexBufferObject);
}

static float no_repeat_vertices[] = {
    -0.6f, -0.5f, 0.0f, // 左下
    0.6f, -0.5f, 0.0f, // 右下
    -0.6f,  0.5f, 0.0f, // 左上
    0.6f,  0.5f, 0.0f // 右上
};

static GLuint indices[] = {
    0, 1, 2,
    2, 3, 1
};

void Renderer::renderEBO() {
    // 各三角形的点可能重复，因此使用去重的点数组，然后额外描述一个三角形用的哪些点下标
    // 保存下标的地方是 element buffer object
    // 用 vao 保存 ebo，需要在 bind vao 之后调用 bind ebo
   
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    GLuint vertexBufferObject;
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(no_repeat_vertices),
                 no_repeat_vertices, GL_STATIC_DRAW);
    // 配置会保存到 vao
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);
    // 把 ebo 保存到 vao
    GLuint elementBuffer;
    glGenBuffers(1, &elementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    auto shader = use_simple_shader();
    
    // 画多边形的模式，线框
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    // draw with EBO，参数：元素数、index 数据类型、indices 指针
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    glDeleteProgram(shader);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &elementBuffer);
}

static GLuint use_simple_shader() {
    // shader 操作流程
    // 1 创建 shader，指定类型。OpenGL 要求 vertex、fragment shader 都得有
    // 2 绑定源码
    // 3 编译 shader 源码
    // 4 查询编译是否成功
    // 5 创建 shader program，链接多个 shader 到 shader program 中
    // 6 查询链接是否成功
    // 7 激活程序，释放 shader
    
    // 1 创建
    auto shader = glCreateShader(GL_VERTEX_SHADER);
    // 2 可以用多个字符串拼接在一起，第四个参数字符串长度列表可以不给
    // 这次绑的代码就是取第 0 个属性，3 个 float 就是 vec3 类型
    glShaderSource(shader, 1, &vertext_shader, 0);
    // 3 编译
    glCompileShader(shader);
    // 4 是否成功
    GLint ok;
    char err[128] = {0};
    // iv 代表返回的是 int vector，第二个参数是要查询的项目
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        // 取失败信息
        glGetShaderInfoLog(shader, 127, 0, err);
        assert(false);
        log(err);
    }
    
    // 1 create
    auto f_shader = glCreateShader(GL_FRAGMENT_SHADER);
    // 2 bind source
    glShaderSource(f_shader, 1, &fragment_shader, 0);
    // 3 compile source
    glCompileShader(f_shader);
    // 4 query result
    glGetShaderiv(f_shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        memset(err, 0, 128);
        glGetShaderInfoLog(f_shader, 127, 0, err);
        assert(false);
        log(err);
    }
    
    // 5 create program
    auto program = glCreateProgram();
    // attach
    glAttachShader(program, shader);
    glAttachShader(program, f_shader);
    // link：上一个 shader 的输出必须是下一个 shader 的输入？？？
    glLinkProgram(program);
    // 6 query
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        memset(err, 0, 128);
        glGetProgramInfoLog(program, 127, 0, err);
        assert(false);
        log(err);
    }
    // 7 activate
    glUseProgram(program);
    glDeleteShader(shader);
    glDeleteShader(f_shader);
    return program;
}

void log(const char *fmt, ...) {
    
}

// MARK: - 下面是练习

static float vertices_3_1[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f, 0.5f, 0.0f
};

static float vertices_3_2[] = {
    0.6f, 0.6f, 0.0f,
    0.9f, 0.6f, 0.0f,
    0.75f, 0.9f, 0.0f
};

static GLuint use_simple_shader_2();

void Renderer::renderVAO2() {
    // vao 具体会保存：attribute pointer 及其关联的 vbo，enable/disable
    GLuint vaos[2];
    glGenVertexArrays(2, vaos);

    GLuint vbos[2];
    glGenBuffers(2, vbos);
    
    glBindVertexArray(vaos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_3_1), vertices_3_1, GL_STATIC_DRAW);
    // 配置会保存到 vao
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(vaos[1]);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_3_2), vertices_3_2, GL_STATIC_DRAW);
    // 配置会保存到 vao
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);
    
    auto shader = use_simple_shader();
    
    glBindVertexArray(vaos[0]);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    glDeleteProgram(shader);
    shader = use_simple_shader_2();

    glUseProgram(shader);
    glBindVertexArray(vaos[1]);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteProgram(shader);
    glBindVertexArray(0);
    glDeleteVertexArrays(2, vaos);
    glDeleteBuffers(2, vbos);
}

static GLuint use_simple_shader_2() {
    
    auto shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader, 1, &vertext_shader, 0);
    glCompileShader(shader);
    GLint ok;
    char err[128] = {0};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        glGetShaderInfoLog(shader, 127, 0, err);
        assert(false);
        log(err);
    }
    auto f_shader = glCreateShader(GL_FRAGMENT_SHADER);
  
    static const char *fragment_shader_2 =
    "   #version 330 core"LF
    "   out vec4 f_color;"LF
    "   void main() {"LF
    "      f_color = vec4(0.0f, 0.5f, 0.75f, 1.0f);"LF
    "   }"LF
    ;

    glShaderSource(f_shader, 1, &fragment_shader_2, 0);
    glCompileShader(f_shader);
    glGetShaderiv(f_shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        memset(err, 0, 128);
        glGetShaderInfoLog(f_shader, 127, 0, err);
        assert(false);
        log(err);
    }
    
    auto program = glCreateProgram();
    glAttachShader(program, shader);
    glAttachShader(program, f_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        memset(err, 0, 128);
        glGetProgramInfoLog(program, 127, 0, err);
        assert(false);
        log(err);
    }
    glUseProgram(program);
    glDeleteShader(shader);
    glDeleteShader(f_shader);
    return program;
}
