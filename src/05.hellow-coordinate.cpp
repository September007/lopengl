#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <helper/helper.h>
#define STB_IMAGE_IMPLEMENTATION
#include <helper/stb_image.h>
#include <map>
using namespace glm;

constexpr int screenWidth = 800, screenHeight = 600;
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

    auto window = glfwCreateWindow(screenWidth, screenHeight, "Learning OpenGL", NULL, NULL);
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
    glViewport(0, 0, screenWidth, screenHeight);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // shader && program
    ShaderObject vs = {GL_VERTEX_SHADER, readFile("../media/shaders/05.hellow-coordinate/this.vs.glsl")};
    ShaderObject fs = {GL_FRAGMENT_SHADER, readFile("../media/shaders/05.hellow-coordinate/this.fs.glsl")};
    // return GL_TRUE , which is 1
    GLint shaderCompileStatus[] = {vs.getInfo(GL_COMPILE_STATUS), fs.getInfo(GL_COMPILE_STATUS)};
    ProgramObject program = Helper::CreateProgram(vs, fs);
    // load texture

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // 8 bit!!!!, if not, use GL_LINEAR instead of GL_NEARST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nChannels;
    // OpenGL image coord.y(y=0) is at bottom ,but image coord.y(y=0) is at top
    stbi_set_flip_vertically_on_load(true);
    auto data = stbi_load("../media/texture/bmp/Rainbow.bmp", &width, &height, &nChannels, 3);
    _ASSERT(data != nullptr);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    GLuint texture2;
    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width2, height2, nChannels2;
    auto data2 = stbi_load("../media/texture/bmp/2004050204170.bmp", &width2, &height2, &nChannels2, 0);
    _ASSERT(data2 != nullptr);
    // see this strange format paramter,since GL_RGBA=GL_RGBA + 1, when nChannels get 4 , the value is equal to GL_RGBA
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB + nChannels2 - 3, width2, height2, 0, GL_RGB + nChannels2 - 3, GL_UNSIGNED_BYTE, data2);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data2);

    GL_ERROR_STOP();
    constexpr auto __L = -1.f, __R = 2.f;
    constexpr auto _L = -0.5f, _R = 0.5f;
    // position color
   float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};
    GLuint indices[] = {
        0, 1, 2,    // triangle one
        0, 1, 3,    // triangle two
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
    _ASSERT(attribitues.find("aPos")!=attribitues.end());
    _ASSERT(attribitues.find("aTexCoord")!=attribitues.end());
    glVertexAttribPointer(attribitues["aPos"].position, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(attribitues["aPos"].position);
    glVertexAttribPointer(attribitues["aTexCoord"].position, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(attribitues["aTexCoord"].position);
   
    // 4. 绑定 EBO //
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    auto uniforms = program.getUniforms();
    auto attributes=program.getAttributes();
    GL_ERROR_STOP();
    // the follow two all work
    glUseProgram(program.getProgram());
    // glUniform1i(glGetUniformLocation(program.getProgram(),"ourTexture1"),0);
    GL_ERROR_STOP();
    program.setInt("ourTexture2", 1);
    GL_ERROR_STOP();
    std::map<int, int> frame_record;
    frame_record[-1] = 0;

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        glfwPollEvents();
        glClearColor(0.2, .2, .3, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program.getProgram());
        double tm = glfwGetTime();

        // coordinate transform
        mat4 model = rotate<float>(mat4(1.f), tm*radians<float>(-55), vec3(0.5, 1.0, 0));
        mat4 view = mat4(1.f);
        view = glm::translate(view, glm::vec3(0.0f, 0.0f,
                                              -3 // sinf(tm)*2
                                              ));
        auto rate = float(screenWidth) / screenHeight;
        mat4 projection = glm::perspective<float>(radians<float>(45), rate, .1, 100);
        projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
        GL_ERROR_STOP();
        program.setFloat("mixRate", sinf(tm) / 2 + 0.5);
        int model_loc = program.getUniformLocation("model");
        int view_loc = program.getUniformLocation("view");
        int proj_loc = program.getUniformLocation("projection");
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, value_ptr(model));
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(proj_loc, 1, GL_FALSE, value_ptr(projection));
        GL_ERROR_STOP();
        {
            vec4 vec(1.f, 1.f, 0, 1);
            vec4 vecs[4] = {
                projection * view * model * vec,
                view * model * vec,
                model * vec,
                vec};
            for (auto &v : vecs)
                std::cout << to_string(v) << std::endl;
            //_ASSERT(!ProjectionOutOfRange(vec,projection,view,model));
        }
        glBindVertexArray(VAO);
        //glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, 6 + (GLuint *)0);
        glDrawArrays(GL_TRIANGLES,0,36);
        glfwSwapBuffers(window);
    }
    return 0;
}