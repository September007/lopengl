#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <helper/helper.h>
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}
int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    auto window = glfwCreateWindow(800, 600, "Learning OpenGL", NULL, NULL);
    if (!window)
    {
        std::cout << "create widnow failed" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "glad load failed" << std::endl;
        return -1;
    }
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // shader && program
    ShaderObject vs = {GL_VERTEX_SHADER, readFile("../media/shaders/01.hellow-triangle/this.vs.glsl")};
    ShaderObject fs = {GL_FRAGMENT_SHADER, readFile("../media/shaders/01.hellow-triangle/this.fs.glsl")};
    // return GL_TRUE , which is 1
    GLint shaderCompileStatus[] = {vs.getInfo(GL_COMPILE_STATUS), fs.getInfo(GL_COMPILE_STATUS)};
    ProgramObject program = Helper::CreateProgram(vs, fs);
    GLint programCompileStatus = program.getInfo(GL_COMPILE_STATUS);

    GLfloat vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f, 0.5f, 0.0f};
    // VBO
    GLuint VBO, VAO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // 1. 绑定VAO
    glBindVertexArray(VAO);
    // 2. 把顶点数组复制到缓冲中供OpenGL使用
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // 3. 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    GLfloat vertices1[] = {
        -0.5f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
        0.0f, -0.5f, 0.0f};
    // VBO
    GLuint VBO1, VAO1;

    glGenVertexArrays(1, &VAO1);
    glGenBuffers(1, &VBO1);

    // 1. 绑定VAO
    glBindVertexArray(VAO1);
    // 2. 把顶点数组复制到缓冲中供OpenGL使用
    glBindBuffer(GL_ARRAY_BUFFER, VBO1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices1, GL_STATIC_DRAW);
    // 3. 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        glfwPollEvents();
        glClearColor(0.2, .2, .3, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program.getProgram());
        int tm = glfwGetTime();
        if ((tm / 3) & 1)
            glBindVertexArray(VAO);
        else
            glBindVertexArray(VAO1);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
    }
    return 0;
}