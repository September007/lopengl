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

// primitive attribute, this will be trans into shader for distinguising current-handling primitive
#define PRIMITIVE_NONE  0
#define PRIMITIVE_CIRCLE_POINT 1
#define PRIMITIVE_LINE 2

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
    {// set camera position
        camera.cameraPos=vec3(0,0,2);
        
    }
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
    ShaderObject draw_line_fs = {GL_FRAGMENT_SHADER, readFile("../media/shaders/" + getSrcFileNameOnlyName(__FILE__) + "/draw_axiss.fs.glsl")};
    
    ProgramObject program = Helper::CreateProgram(vs, fs);
    ProgramObject draw_line_program=Helper::CreateProgram(vs,draw_line_fs);
    // load texture
    GLuint texture;
    TextureObject t1 = Helper::CreateTexture(GL_TEXTURE0, "../media/texture/bmp/skiing.dib");
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
    constexpr auto _L = -0.f, _R = 0.3f, _D = 0.0f;
    constexpr auto _LL = -0.3f, _RR = 0.3f;
    constexpr auto _L1=-0.8f,_R1=0.0f,_B1=-0.2f,_T1=-_B1;
    constexpr auto _L2=-0.0f,_R2=0.8f,_B2=-0.2f,_T2=-_B2;
    // position : 3     color : 2       primitive : 1
    float vertices[] = {
        _L1, _B1, _D, __L, __L,PRIMITIVE_NONE,
        _R1, _B1, _D, __R, __L,PRIMITIVE_NONE,
        _R1, _T1, _D, __R, __R,PRIMITIVE_NONE,
        _L1, _T1, _D, __L, __R,PRIMITIVE_NONE,

        _L2, _B2, _D, __L, __L,PRIMITIVE_NONE,
        _R2, _B2, _D, __R, __L,PRIMITIVE_NONE,
        _R2, _T2, _D, __R, __R,PRIMITIVE_NONE,
        _L2, _T2, _D, __L, __R,PRIMITIVE_NONE,
        // debugging O(0,0,0) X(1,0,0) Y(0,1,0)
        // line 
        0,0,0, 0,0, PRIMITIVE_LINE,
        1,0,0, 0,0, PRIMITIVE_LINE,
        0,1,0, 0,0, PRIMITIVE_LINE,
        // point
        0,0,0, 0,0, PRIMITIVE_CIRCLE_POINT,
        1,0,0, 0,0, PRIMITIVE_CIRCLE_POINT,
        0,1,0, 0,0, PRIMITIVE_CIRCLE_POINT,
        };
    GLuint indices[] = {
        0, 1, 2,    // left triangle one
        0, 2, 3,    // left triangle two
        4, 5, 6,    //right triangle one
        4, 6, 7,    //right triangle two
        8,9,8,10
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
    //glVertexAttribPointer(attribitues["aChooseTex"].position, 1, GL_FLOAT, GL_FALSE, strip * sizeof(float), (void *)(5 * sizeof(float)));
    //glEnableVertexAttribArray(attribitues["aChooseTex"].position);

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
    program.setInt("ourTexture1", 0);
    GL_ERROR_STOP();
    std::map<int, int> frame_record;
    frame_record[-1] = 0;

    // depth test && SSAA anti-aliasing
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window))
    {
        double tm = glfwGetTime();
        camera.Tick(tm);
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
                                              -1 // sinf(tm)*2
                                              ));
        auto rate = float(screenWidth) / screenHeight;
        auto projection = camera.GetProjectionMatrix();
        GL_ERROR_STOP();
        // shut camera view-changing
        //model=projection= view=mat4(1.0f);
        //
        program.setFloat("mixRate", sinf(tm) / 2 + 0.5);
        int model_loc = program.getUniformLocation("model");
        int view_loc = program.getUniformLocation("view");
        int proj_loc = program.getUniformLocation("projection");
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, value_ptr(model));
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(proj_loc, 1, GL_FALSE, value_ptr(projection));
        GL_ERROR_STOP();

        glBindVertexArray(VAO);
        // left rectangle
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 3 + (GLuint *)0);
        // right rectangle
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 6 + (GLuint *)0);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 9 + (GLuint *)0);

        GL_ERROR_STOP();



        // draw O,X,Y   see shader: O:black, X:red, Y:green
        glPointSize(20);
        model=glm::translate(glm::mat4(1.0f),glm::vec3(0,0,0.5));
        glUseProgram(draw_line_program.getProgram());
        auto d_attributes=draw_line_program.getAttributes();
        auto d_uniforms=draw_line_program.getUniforms();
        _ASSERT(d_uniforms.find("model")!=d_uniforms.end());
        _ASSERT(d_uniforms.find("view")!=d_uniforms.end());
        _ASSERT(d_uniforms.find("projection")!=d_uniforms.end());
        _ASSERT(d_uniforms.find("primitive_id")!=d_uniforms.end());
        glUniformMatrix4fv(d_uniforms["model"].position, 1, GL_FALSE, value_ptr(model));
        glUniformMatrix4fv(d_uniforms["view"].position, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(d_uniforms["projection"].position, 1, GL_FALSE, value_ptr(projection));
        glBindVertexArray(VAO);
        glLineWidth(10);
        // X-axis + Y-axis
        glUniform1i(d_uniforms["primitive_id"].position,PRIMITIVE_LINE);
        glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 12 + (GLuint *)0);
        glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 14 + (GLuint *)0);
        // O + X + Y
        glUniform1i(d_uniforms["primitive_id"].position,PRIMITIVE_CIRCLE_POINT);
        glDrawElements(GL_POINTS, 3, GL_UNSIGNED_INT, 13 + (GLuint *)0);
        
        GL_ERROR_STOP();
        
        
        glfwSwapBuffers(window);
    }
    return 0;
}
TEST(l, r)
{
    tmain();
}