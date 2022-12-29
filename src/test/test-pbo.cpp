#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <helper/helper.h>
#include <thread>
#include <chrono>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include <helper/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <helper/stb_image_write.h>
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

constexpr int window_width = 800, window_height = 600;
int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    auto window = glfwCreateWindow(window_width, window_height, "Learning OpenGL", NULL, NULL);
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
    ShaderObject vs = {GL_VERTEX_SHADER, R"(#version 330 core

layout(location=0)in vec3 aPos;
layout(location=1)in vec3 aCol;

out vec2 texCoord;
uniform vec4 offset;
out vec4 color;
void main(){
    texCoord=gl_Position=vec4(aPos,1)+offset;
    color=vec4(aCol,1.f);
})"};
    ShaderObject fs = {GL_FRAGMENT_SHADER, R"(#version 330
in vec2 texCoord;
uniform sampler2D tex;
void main(){
    FragColor=texture(tex,texCoord);
}
    )"};
    // return GL_TRUE , which is 1
    GLint shaderCompileStatus[] = {vs.getInfo(GL_COMPILE_STATUS), fs.getInfo(GL_COMPILE_STATUS)};
    ProgramObject program = Helper::CreateProgram(vs, fs);

    GLfloat vertices[] = {
        -0.5f, -0.5f, 0.0f, 1.f, 0.f, 0.f, // left bottom
        0.5f, -0.5f, 0.0f, .0f, 1.f, 0.f,  // right bottom
        0.5f, 0.5f, 0.0f, .0f, .0f, 1.f,   // right top
        -0.5f, 0.5f, 0.0f, .5f, .5f, .5f   // left top
    };

    GLuint indices[] = {
        0, 1, 2, // triangle one
        0, 1, 3  // triangle two
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
    // 3. 设置顶点属性指针 //
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // 4. 绑定 EBO //
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    /* triangle index */
    int lastIndex = 0;
    /* packing pbo */
    constexpr int numof_pboIds = 2;
    GLuint pboIds[numof_pboIds];
    glGenBuffers(numof_pboIds, pboIds);
    for (int i = 0; i < numof_pboIds; ++i)
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[i]);
        glBufferData(GL_PIXEL_PACK_BUFFER, window_width * window_height, 0, GL_DYNAMIC_READ);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }
    /* unpacking pbo, to provide image data*/;
    constexpr int numof_upboIds = 2;
    GLuint upboIds[numof_pboIds];
    glGenBuffers(numof_upboIds, upboIds);
    constexpr char *ifiles[] = {
        "../media/texture/bmp/2004050204170.bmp",
        "../media/texture/bmp/Rainbow.bmp"};
    stbi_uc *image_datas[numof_upboIds];
    struct image_info
    {
        int width, height, nChannels;
    } iinfos[2];
    for (int i = 0; i < numof_upboIds; ++i)
    {
        image_datas[i] = stbi_load(ifiles[i], &iinfos[i].width, &iinfos[i].height, &iinfos[i].nChannels, 3);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, upboIds[i]);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, iinfos[i].width * iinfos[i].height * iinfos[i].nChannels, image_datas[i], GL_DYNAMIC_COPY);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }
    GLuint texObj;
    glGenTextures(1,&texObj);
    glBindTexture(GL_TEXTURE_2D,texObj);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXUTURE_2D,0,GL_RGB,1,1,0,GL_RGB,GL_UNSIGNED_BYTE,0);// data will be bind after
    glBindTexture(GL_TEXTURE_2D,0);
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        glfwPollEvents();
        glClearColor(0.2, .2, .3, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        double tm = glfwGetTime();
        int itm = tm;

        GLfloat offset[] = {sinf(tm) / 4, cosf(tm) / 4, 0, 0};
        glUseProgram(program.getProgram());

        auto uniforms = program.getUniforms();
        glUniform4f(program.getUniformLocation("offset"), sinf(tm) / 4, cosf(tm) / 4, 0, 0);
        program.setInt("tex",texObj);

        glBindVertexArray(VAO);
        if (itm & 2)
        {
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
        }
        else
        {
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 3 + (GLuint *)0);
        }
        glBindBuffer()
        glfwSwapBuffers(window);
    }

    return 0;
}