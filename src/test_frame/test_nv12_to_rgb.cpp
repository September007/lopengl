#include <gtest/gtest.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <helper/OpenGL_Utils.h>
#include <map>

using glm::mat4;
using glm::min;
using glm::radians;
using glm::rotate;
using glm::vec3;

constexpr int screenWidth = 800, screenHeight = 600;

CameraWithController camera(screenWidth, screenHeight);

void mouseCallback(GLFWwindow *window, double xpos, double ypos)
{
    camera.mouseCallback(xpos, ypos);
}
void scrollCallback(GLFWwindow *window, double offsetX, double offsetY)
{
    camera.scrollCallback(offsetX, offsetY);
}
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    camera.framebuffer_size_callback(window, width, height);
    glViewport(0, 0, width, height);
}
void processInput(GLFWwindow *window)
{
    camera.processInput(window);
}

int tmain()
{
    constexpr char *p = __FILE__;
    glfwInit();

    glfwWindowHint(GL_SAMPLES, 8);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    auto window = glfwCreateWindow(screenWidth, screenHeight, "Learning OpenGL", NULL, NULL);
    if (!window)
    {
        std::cout << "create widnow failed" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "glad load failed" << std::endl;
        return -1;
    }
    glViewport(0, 0, screenWidth, screenHeight);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    // shader && program
    ShaderObject vs = {GL_VERTEX_SHADER, readFile("../media/shaders/" + getSrcFileNameOnlyName(__FILE__) + "/this.vs.glsl")};
    ShaderObject fs = {GL_FRAGMENT_SHADER, readFile("../media/shaders/" + getSrcFileNameOnlyName(__FILE__) + "/this.fs.glsl")};
    ProgramObject program = Helper::CreateProgram(vs, fs);

    // load texture
    GLuint texture;
    TextureObject t1 = Helper::CreateTexture(GL_TEXTURE0, "../media/texture/bmp/Rainbow.bmp");
    texture = t1.GetTTexture();

    GLuint texture2;
    TextureObject t2 = Helper::CreateTexture(GL_TEXTURE1, "../media/texture/bmp/2004050204170.bmp");
    texture2 = t2.GetTTexture();

    GL_ERROR_STOP();

    auto intAsFloat=[](int32_t i){
        float ret=*reinterpret_cast<float*>(&i);
        return ret;
    };
    constexpr auto __L = 1.f, __R = -1.f;
    constexpr auto _L = -0.3f, _R = 0.3f, _D = 0.0f;
    // position color
    float vertices[] = {
        _L, _L, _D, __L, __L,intAsFloat(0),
        _R, _L, _D, __R, __L,intAsFloat(0),
        _R, _R, _D, __R, __R,intAsFloat(0),
        _L, _R, _D, __L, __R,intAsFloat(0),

        _L, _L, _D, __L, __L,intAsFloat(1),
        _R, _L, _D, __R, __L,intAsFloat(1),
        _R, _R, _D, __R, __R,intAsFloat(1),
        _L, _R, _D, __L, __R,intAsFloat(1),
        };
    GLuint indices[] = {
        0, 1, 2,    // triangle one
        0, 2, 3,    // triangle two
        0, 1, 2, 3, // rectangle
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
    auto attribitues = getAttributes(program.getProgram());
    // 3. 设置顶点属性指针 //
    _ASSERT(attribitues.find("aPos") != attribitues.end());
    _ASSERT(attribitues.find("aTexCoord") != attribitues.end());
    int strip = 6;
    glVertexAttribPointer(attribitues["aPos"].position, 3, GL_FLOAT, GL_FALSE, strip * sizeof(float), (void *)0);
    glEnableVertexAttribArray(attribitues["aPos"].position);
    glVertexAttribPointer(attribitues["aTexCoord"].position, 2, GL_FLOAT, GL_FALSE, strip * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(attribitues["aTexCoord"].position);
    glVertexAttribPointer(attribitues["aChooseTex"].position, 1, GL_FLOAT, GL_FALSE, strip * sizeof(float), (void *)(5 * sizeof(float)));
    glEnableVertexAttribArray(attribitues["aChooseTex"].position);

    // 4. 绑定 EBO //
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    auto uniforms = program.getUniforms();
    auto attributes = program.getAttributes();
    GL_ERROR_STOP();
    // the follow two all work
    glUseProgram(program.getProgram());
    // glUniform1i(glGetUniformLocation(program.getProgram(),"ourTexture1"),0);
    GL_ERROR_STOP();
    program.setInt("ourTexture2", 1);
    GL_ERROR_STOP();
    std::map<int, int> frame_record;
    frame_record[-1] = 0;

    // depth test && SSAA anti-aliasing
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window))
    {
        double tm = glfwGetTime();
        camera.Tick(0);
        processInput(window);
        glfwPollEvents();
        glClearColor(0.2, .2, .3, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program.getProgram());
        // coordinate transform
        mat4 model = glm::mat4(1.0f);
       // model = rotate<float>(mat4(1.f), tm * radians<float>(-55), vec3(0.5, 1.0, 0));
        mat4 view = camera.GetViewMatrix();
        view = glm::translate(view, glm::vec3(0.0f, 0.0f,
                                              -3 // sinf(tm)*2
                                              ));
        auto rate = float(screenWidth) / screenHeight;
        auto projection = camera.GetProjectionMatrix();
        GL_ERROR_STOP();
        program.setFloat("mixRate", sinf(tm) / 2 + 0.5);
        int model_loc = program.getUniformLocation("model");
        int view_loc = program.getUniformLocation("view");
        int proj_loc = program.getUniformLocation("projection");
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, value_ptr(model));
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(proj_loc, 1, GL_FALSE, value_ptr(projection));
        GL_ERROR_STOP();

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 3 + (GLuint *)0);

        glfwSwapBuffers(window);
    }
    return 0;
}
TEST(l, r)
{
    tmain();
}