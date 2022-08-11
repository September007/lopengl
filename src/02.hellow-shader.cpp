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
    ShaderObject vs = {GL_VERTEX_SHADER, readFile("../media/shaders/02.hellow-shader/this.vs.glsl")};
    ShaderObject fs = {GL_FRAGMENT_SHADER, readFile("../media/shaders/02.hellow-shader/this.fs.glsl")};
    // return GL_TRUE , which is 1
    GLint shaderCompileStatus[] = {vs.getInfo(GL_COMPILE_STATUS), fs.getInfo(GL_COMPILE_STATUS)};
    ProgramObject program = Helper::CreateProgram(vs, fs);
   
    GLfloat vertices[] = {
        -0.5f, -0.5f, 0.0f,   1.f, 0.f, 0.f,       // left bottom
        0.5f, -0.5f, 0.0f,   .0f, 1.f, 0.f,       // right bottom
        0.5f, 0.5f, 0.0f,    .0f, .0f, 1.f,       // right top
        -0.5f, 0.5f, 0.0f,     .5f,.5f,.5f       // left top
    };

    GLuint indices[] = {
        0, 1, 2, // triangle one
        0,1, 3  // triangle two
    };
    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // 1. 绑定VAO//
    glBindVertexArray(VAO);
    // 2. 把顶点数组复制到缓冲中供OpenGL使用 //
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    auto attribitues=getAttributes(program.getProgram());
    // 3. 设置顶点属性指针 //
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // 4. 绑定 EBO //
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        glfwPollEvents();
        glClearColor(0.2, .2, .3, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        double tm = glfwGetTime();
        int itm=tm;

        GLfloat offset[]={sinf(tm)/4,cosf(tm)/4,0,0};
        glUseProgram(program.getProgram());

        auto uniforms=program.getUniforms();
        glUniform4f(program.getUniformLocation("offset"),sinf(tm)/4,cosf(tm)/4,0,0);

        glBindVertexArray(VAO);
        if (itm & 2)
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
        else
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 3 + (GLuint *)0);

        glfwSwapBuffers(window);
    }
    return 0;
}