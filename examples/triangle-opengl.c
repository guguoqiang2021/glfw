//   
//   变更点： 1： mouse移动到顶点附近按下鼠标按钮，可以移动顶点
//           2：在三角形的外接矩形内部按下鼠标按钮，可以平移整个三角形
//           3：在三角形的外接矩形外部按下鼠标按钮，可以翻转三角形，以X轴或Z轴翻转
//  实际执行效果，参照根目录的Traiangle.gif文件( https://github.com/guguoqiang2021/glfw/edit/master/Traiangle.gif )
//
#define _USE_MATH_DEFINES
#
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "linmath.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

typedef struct Vertex
{
    vec3 pos;
    vec3 col;
} Vertex;

typedef enum _HitType
{
    nop,
    InTriangle,
    OutofTriangle,
    HitVertex1 = 10,
    HitVertex2,
    HitVertex3,

}HitType;

static Vertex vertices[3] =
{
    { { -0.6f, -0.4f,0.0f }, { 1.f, 0.f, 0.f } },
    { {  0.6f, -0.4f,0.0f }, { 0.f, 1.f, 0.f } },
    { {   0.f,  0.6f,0.0f }, { 0.f, 0.f, 1.f } }
};

static const char* vertex_shader_text =
"#version 330\n"
"uniform mat4 MVP;\n"
"in vec3 vCol;\n"
"in vec3 vPos;\n"
"out vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 1.0);\n"
"    color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"#version 330\n"
"in vec3 color;\n"
"out vec4 fragment;\n"
"void main()\n"
"{\n"
"    fragment = vec4(color, 1.0);\n"
"}\n";

HitType gOperation = nop;

GLfloat rotateZ = 0.f, rotateX = 0.f;
GLfloat daltX = 0.f, daltY = 0.f;

double cursorX = 0;
double cursorY = 0;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;

    if (action == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwGetCursorPos(window, &cursorX, &cursorY);
    }
    else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        gOperation = nop;
    }
}

HitType Checkhit(GLFWwindow* window, int cursorX, int cursorY)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float VirualHitX = (cursorX - width / 2.0) / width*2*1.55;
    float VirualHitY = (-cursorY + height / 2.0) / height*2 *1.17;
    double distance;
    float minX=1, minY=1, maxX=-1, maxY=-1;
    for (int iLp = 0; iLp < sizeof(vertices) / sizeof(vertices[0]); iLp++) {
        distance = hypot(VirualHitX - vertices[iLp].pos[0], VirualHitY - vertices[iLp].pos[1]);
        if (distance < 0.03)
            return HitVertex1+iLp;

        if (minX > vertices[iLp].pos[0])
            minX = vertices[iLp].pos[0];
        if (maxX < vertices[iLp].pos[0])
            maxX = vertices[iLp].pos[0];
        if (minY > vertices[iLp].pos[1])
            minY = vertices[iLp].pos[1];
        if (maxY < vertices[iLp].pos[1])
            maxY = vertices[iLp].pos[1];
    }
    if (VirualHitX<maxX && VirualHitX >=minX && VirualHitY <maxY && VirualHitY >minY)
        return InTriangle;
    else
        return OutofTriangle;
}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    {
        HitType hit;
        if (gOperation == nop) {
            hit = Checkhit(window, cursorX, cursorY);
            gOperation = hit;
        }
        else {
            hit = gOperation;
        }
        switch (hit) {
        case OutofTriangle:
            rotateZ += (GLfloat)(x - cursorX) / 10.f;
            rotateX += (GLfloat)(y - cursorY) / 10.f;
            break;
        case InTriangle:
            daltX += (GLfloat)(x - cursorX) / 100.f;
            daltY -= (GLfloat)(y - cursorY) / 100.f;
            break;
        case HitVertex1:
        case HitVertex2:
        case HitVertex3: {
            GLfloat X = (GLfloat)(x - cursorX) / 100.f;
            GLfloat Y = (GLfloat)(y - cursorY) / 100.f;
            vertices[hit - HitVertex1].pos[0] += X;
            vertices[hit - HitVertex1].pos[1] -= Y;
            }
            break;
        }
        cursorX = x;
        cursorY = y;
    }
}

int main(void)
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Triangle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    // NOTE: OpenGL error checks have been omitted for brevity
    const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    const GLint mvp_location = glGetUniformLocation(program, "MVP");
    const GLint vpos_location = glGetAttribLocation(program, "vPos");
    const GLint vcol_location = glGetAttribLocation(program, "vCol");

    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        mat4x4 mvp,m,p;
        mat4x4_identity(m);
        mat4x4_translate(m, daltX, daltY, -2.0f);
        mat4x4_rotate_X(m, m, rotateX);
        mat4x4_rotate_Z(m, m, rotateZ);
        mat4x4_perspective(p, 60.f * (float)M_PI / 180.f,ratio,1.f, 50.f);
        mat4x4_mul(mvp, p, m);

        GLuint vertex_buffer;
        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        GLuint vertex_array;
        glGenVertexArrays(1, &vertex_array);
        glBindVertexArray(vertex_array);
        glEnableVertexAttribArray(vpos_location);
        glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), (void*)offsetof(Vertex, pos));
        glEnableVertexAttribArray(vcol_location);
        glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), (void*)offsetof(Vertex, col));

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp);
        glBindVertexArray(vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

