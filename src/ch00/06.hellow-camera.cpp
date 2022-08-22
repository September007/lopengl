#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <helper/helper.h>
#define STB_IMAGE_IMPLEMENTATION
#include <helper/stb_image.h>
#include <map>

using glm::mat4;
using glm::min;
using glm::radians;
using glm::rotate;
using glm::vec3;

constexpr int screenWidth = 800, screenHeight = 600;
// camera variables, application like 'glm::lookAt(cameraPos, cameraFront + cameraPos, cameraUp);'
glm::vec3 cameraPos = vec3(0, 0, 3);
glm::vec3 cameraFront = vec3(0, 0, -1);
glm::vec3 cameraUp = vec3(0, 1, 0);

float cameraSpeed = 0.5;
// direction && mouse input
float pitch = 0;
float yaw = -90;
float roll = 0;
float lastX = screenWidth / 2, lastY = screenHeight / 2;
// the duration between last two frame, in seconds, this will be set in render loop
float deltaTime = 0;
float lastFrameTime = 0;

float fov = 40;
// this is for debuging pause balahbalh to clip deltaTime to a normal value
float constexpr deltaTimeClipMax = 0.05;
void mouseCallback(GLFWwindow *window, double xpos, double ypos)
{
    static bool first = true;
    auto offsetX = xpos - lastX;
    auto offsetY = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;
    if (first)
    {
        first = false;
        return;
    }

    float sensitivity = 0.05;
    offsetX *= sensitivity;
    offsetY *= sensitivity;

    yaw += offsetX;
    pitch += offsetY;
    // pitch=clamp(pitch,-89,89);
    auto cp = cos(glm::radians<float>(::pitch));
    auto cy = cos(glm::radians<float>(::yaw));
    glm::vec3 front;
    front.x = cos(glm::radians<float>(::pitch)) * cos(glm::radians<float>(::yaw));
    front.y = sin(glm::radians<float>(::pitch));
    front.z = cos(glm::radians<float>(::pitch)) * sin(glm::radians<float>(::yaw));

    auto nf = glm::normalize(front);
    cameraFront = nf;
}
void scrollCallback(GLFWwindow *window, double offsetX, double offsetY)
{
    if (fov >= 1.0 && fov <= 45)
        fov += offsetY;
    fov = clamp(fov, 1, 45);
}
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void processInput(GLFWwindow *window)
{
    auto speed = cameraSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraFront * speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraFront * speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(cross(cameraFront, cameraUp)) * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(cross(cameraFront, cameraUp)) * speed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraSpeed -= deltaTime * 1;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameraSpeed += deltaTime * 1;
}
int main()
{
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
    constexpr auto __L = -1.f, __R = 1.f;
    constexpr auto _L = -0.3f, _R = 0.3f;
    // position color
    float vertices[] = {
        _L, _L, _L, __L, __L,
        _R, _L, _L, __R, __L,
        _R, _R, _L, __R, __R,
        _R, _R, _L, __R, __R,
        _L, _R, _L, __L, __R,
        _L, _L, _L, __L, __L,

        _L, _L, _R, __L, __L,
        _R, _L, _R, __R, __L,
        _R, _R, _R, __R, __R,
        _R, _R, _R, __R, __R,
        _L, _R, _R, __L, __R,
        _L, _L, _R, __L, __L,

        _L, _R, _R, __R, __L,
        _L, _R, _L, __R, __R,
        _L, _L, _L, __L, __R,
        _L, _L, _L, __L, __R,
        _L, _L, _R, __L, __L,
        _L, _R, _R, __R, __L,

        _R, _R, _R, __R, __L,
        _R, _R, _L, __R, __R,
        _R, _L, _L, __L, __R,
        _R, _L, _L, __L, __R,
        _R, _L, _R, __L, __L,
        _R, _R, _R, __R, __L,

        _L, _L, _L, __L, __R,
        _R, _L, _L, __R, __R,
        _R, _L, _R, __R, __L,
        _R, _L, _R, __R, __L,
        _L, _L, _R, __L, __L,
        _L, _L, _L, __L, __R,

        _L, _R, _L, __L, __R,
        _R, _R, _L, __R, __R,
        _R, _R, _R, __R, __L,
        _R, _R, _R, __R, __L,
        _L, _R, _R, __L, __L,
        _L, _R, _L, __L, __R};

    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f, 2.0f, -2.5f),
        glm::vec3(1.5f, 0.2f, -1.5f),
        glm::vec3(-1.3f, 1.0f, -1.5f)};
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
    _ASSERT(attribitues.find("aPos") != attribitues.end());
    _ASSERT(attribitues.find("aTexCoord") != attribitues.end());
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
        deltaTime = min<float>(deltaTimeClipMax, tm - lastFrameTime);
        lastFrameTime = tm;
        processInput(window);
        glfwPollEvents();
        glClearColor(0.2, .2, .3, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program.getProgram());
        // coordinate transform
        mat4 model = rotate<float>(mat4(1.f), tm * radians<float>(-55), vec3(0.5, 1.0, 0));
        mat4 view = glm::lookAt(cameraPos, cameraFront + cameraPos, cameraUp);
        view = glm::translate(view, glm::vec3(0.0f, 0.0f,
                                              -3 // sinf(tm)*2
                                              ));
        auto rate = float(screenWidth) / screenHeight;
        auto projection = glm::perspective(glm::radians(fov), (float)width / (float)height, 0.1f, 100.0f);
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

        glm::mat4 baseRotation = glm::rotate(glm::mat4(1.0f), glm::radians<float>(tm * 19), glm::vec3(1.0, 0, 0));
        for (int i = 0; i < 10; ++i)
        {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), cubePositions[i]);
            model = rotate<float>(model, glm::radians<float>(20.f * i), glm::vec3(1.0, 0.3, 0.5));
            model = model * baseRotation;
            glUniformMatrix4fv(program.getUniformLocation("model"), 1, GL_FALSE, value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
    }
    return 0;
}