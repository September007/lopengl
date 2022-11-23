#pragma once
#include <glad/glad.h>

#include <buffer.hpp>
#include <context.hpp>
#include <GLFW/glfw3.h>
#include <helper/error.h>
#include <helper/msg_center.h>
#include <helper/macro_util.h>
#include <thread>
using namespace std::literals;
template <bool strict_ = STRICT_>
struct ShaderObject
{
    // register callback
    inline static const ScopeObject registerCallback = {
        []
        {
            using namespace error_map;
            Helper::Register(code_packet(error_code::error, object_code::shader, message_code::compile),
                             [] {

                             });
        },
        [] {

        }};
    std::shared_ptr<GLint> shaderHandler = nullptr;
    // save state info which queried last time, the construction func would set this as compile info
    std::string tempInfo;
    ShaderObject(GLint shader_type, std::string src)
    {
        shaderHandler = std::shared_ptr<GLint>(new GLint(glCreateShader(shader_type)),
                                               [](GLint *hand)
                                               {
                                                   if (*hand)
                                                       glDeleteShader(*hand);
                                                   delete hand;
                                               });
        const char *const srcs[] = {src.c_str()};
        glShaderSource(*shaderHandler, 1, srcs, NULL);
        glCompileShader(*shaderHandler);
        int success;
        // get compile info
        int len;
        char infoLog[512];
        glGetShaderiv(*shaderHandler, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(*shaderHandler, 512, &len, infoLog);
            tempInfo = {infoLog, infoLog + len};
            if constexpr (strict_)
            {
                throw std::runtime_error("shader compile failed:\n\t" + tempInfo);
            }
        }
    }
    auto Attach(GLint targetProgram) const -> void
    {
        return glAttachShader(targetProgram, *shaderHandler);
    }
    auto getInfo(GLenum pname)
    {
        constexpr int tempLength = 1024;
        char temp[tempLength];
        GLint params = GL_FALSE;
        glGetShaderiv(*shaderHandler, pname, &params);
        GLsizei len;
        glGetShaderInfoLog(*shaderHandler, tempLength, &len, temp);
        tempInfo = {temp, temp + len};
        return params;
    }
};

// pre declaration
namespace Helper
{
    template <typename... MultiShaderObj>
    inline GLint CreateProgram(MultiShaderObj... mso);
}
using GLHandle = std::shared_ptr<GLint>;
template <bool strict_ = STRICT_>
struct ProgramObject
{
    GLHandle program = nullptr;
    // save state info recently queried
    std::string tempInfo;
    ProgramObject(GLint program = 0)
    {
        reset(program);
    }
    ProgramObject(const std::string &vsFileName, const std::string &fsFileName)
    {
        reset(Helper::CreateProgram(ShaderObject(GL_VERTEX_SHADER, readFile(vsFileName)), ShaderObject(GL_FRAGMENT_SHADER, readFile(fsFileName))));
    }
    void reset(GLint program)
    {
        this->program.reset(new GLint(program), [](GLint *p)
                            { glDeleteProgram(*p); });
        if (program == 0)
            return;
        // check program link status
        auto ret = getInfo(GL_LINK_STATUS);
        if constexpr (strict_)
            if (ret != GL_TRUE)
                throw std::runtime_error("program link failed:\n\t" + tempInfo);
    }
    DebugArea(static inline std::atomic<int> use_cnt = 0;);
    void use()
    {
        DebugArea(ASSERT(use_cnt.load() == 0));
        glUseProgram(getProgram());
        DebugArea(use_cnt++;);
    }
    void unuse()
    {
        DebugArea(ASSERT(use_cnt == 1));
        glUseProgram(0);
        DebugArea(use_cnt--);
    }
    [[nodiscard]] auto temp_use()
    {
        // here could report something
        return ScopeObject([this]
                           { this->use(); },
                           [this]
                           { this->unuse(); });
    }
    GLint getProgram() const { return *program; }
    auto getInfo(GLenum pname)
    {
        if (getProgram() == 0)
            return GL_FALSE;
        constexpr int tempLength = 1024;
        char temp[tempLength];
        GLint params = GL_FALSE;
        glGetProgramiv(getProgram(), pname, &params);
        GLsizei len;
        glGetProgramInfoLog(getProgram(), tempLength, &len, temp);
        tempInfo = {temp, temp + len};
        return params;
    }
    auto getAttributes() const
    {
        return ::getAttributes(getProgram());
    }
    auto getUniforms() const
    {
        std::map<std::string, AttributeInfo> ret;
        int count = 0;
        glGetProgramiv(getProgram(), GL_ACTIVE_UNIFORMS, &count);
        constexpr int name_length = 256;
        char cache_name[name_length];
        GLint used_cache_name_length = 0;
        for (int i = 0; i < count; ++i)
        {
            AttributeInfo info;
            glGetActiveUniform(getProgram(), GLint(i), name_length, &used_cache_name_length, &info.size, &info.type, cache_name);
            info.name = std::string(cache_name, cache_name + used_cache_name_length);
            info.position = glGetUniformLocation(getProgram(), info.name.c_str());
            ret[info.name] = info;
        }
        return ret;
    }
    // uniform setter
    void setBool(const std::string &name, bool value) const
    {
        DebugArea(if(!checkExist(getUniforms(), name))return);
        std::cerr << fmt::format("{:15} {:<20} {:>10}", "Setbool", name, value) << std::endl;
        glUniform1i(glGetUniformLocation(getProgram(), name.c_str()), (int)value);
    }
    void setInt(const std::string &name, int value) const
    {
        DebugArea(if(!checkExist(getUniforms(), name))return;);
        std::cerr << fmt::format("{:15} {:<20} {:>10}", "SetInt", name, value) << std::endl;
        glUniform1i(glGetUniformLocation(getProgram(), name.c_str()), value);
    }
    void setFloat(const std::string &name, float value) const
    {
        DebugArea(if(!checkExist(getUniforms(), name))return);
        std::cerr << fmt::format("{:15} {:<20} {:>10}", "SetFloat", name, value) << std::endl;
        glUniform1f(glGetUniformLocation(getProgram(), name.c_str()), value);
    }
    int getUniformLocation(const std::string &name)
    {
        return glGetUniformLocation(getProgram(), name.c_str());
    }
    // this is compaticable of Light::Objects

    // use this for location binding
    void prepareVBO(Light::VertexBuffer &vbo)
    {
        auto layout = vbo.getLayout();
        std::vector<std::pair<std::string, GLint>> locas;
        int pos = 0;
        for (auto &e : layout)
        {
            locas.emplace_back(e.getName(), pos++);
        }
        bindAttributesLocations(getProgram(), locas);
    }
    inline bool checkExist(std::map<std::string, AttributeInfo> map, const string &name) const
    {
        auto ex = (map.find(name) != map.end());
        if (!ex)
        {
            std::cerr << fmt::format("{:15} {:<20} {:>10}","NoSet-notFound", name,"") << std::endl;
        }
        return ex;
    }
};

namespace
{
    template <bool isBegin, typename FirstShader, typename... RestShaders>
    GLint _CreateProgram(GLint program, FirstShader fs, RestShaders... rest)
    {
        static_assert(std::is_same_v<FirstShader, ShaderObject<>>, "只接受ShaderObject类型参数");

        if constexpr (isBegin)
        {
            program = glCreateProgram();
        }
        fs.Attach(program);

        if constexpr (sizeof...(RestShaders) == 0)
            return program;
        else
            return _CreateProgram<false>(program, rest...);
    }

    template <int isBegin>
    GLint _CreateProgram(GLint program)
    {
        return program;
    }
} // namespace

namespace Helper
{
    template <typename... MultiShaderObj>
    inline GLint CreateProgram(MultiShaderObj... mso)
    {
        auto program = _CreateProgram<true>(0, std::forward<MultiShaderObj>(mso)...);
        glLinkProgram(program);
        return program;
    }

} // namespace Helper

class TextureObject
{
public:
    std::shared_ptr<GLuint> obj;
    GLuint targetTexture;
    int width, height, nChannels;
    TextureObject() = delete;
    TextureObject(GLuint obj_, GLuint targetTex) : targetTexture(targetTex)
    {
        obj.reset(new GLuint(obj_), [](GLuint *p)
                  {
            if(p)
                glDeleteTextures(1,p);
            delete p; });
    }
    auto GetTTexture() const { return *obj; }
    void SetWHN(int w, int h, int n)
    {
        width = w;
        height = h;
        nChannels = n;
    }
    void bind(GLuint target)
    {
        GL_ERROR_STOP();

        // glBindTexture(target,*obj);
        GL_ERROR_STOP();
    }
};
namespace Helper
{
    // support RGB or RGBA
    inline auto CreateTexture(GLuint textureTarget, void *data, int width, int height, int nChannels)
    {
        GLuint texture;
        glActiveTexture(textureTarget); // binding textureTarget->GL_TEXTURE_2D
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture); // binding GL_TEXTURE_2D->texture, so there is textureTarget->texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        auto ret = TextureObject(texture, textureTarget);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB + nChannels - 3, width, height, 0, GL_RGB + nChannels - 3, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        ret.SetWHN(width, height, nChannels);
        return ret;
    }
    struct stb_pic_data
    {
        void *data = nullptr;
        int w, h, n;
        static auto create_stb_pic_data(const string &filePath)
        {
            std::shared_ptr<stb_pic_data> ret(new stb_pic_data(), [](stb_pic_data *p)
                                              {
                if(p)
                    stbi_image_free(p->data); });
            ret->data = stbi_load(filePath.c_str(), &ret->w, &ret->h, &ret->n, 0);
            return ret;
        }

    private:
        stb_pic_data() {}
        ~stb_pic_data() {}
    };
    inline auto CreateTexture(GLuint textureTarget, const std::string &dataFile)
    {
        static ScopeObject setting = {[]
                                      {
                                          stbi_set_flip_vertically_on_load(true);
                                      },
                                      nullptr};

        // int width, height, nChannels;
        // auto data = stbi_load(dataFile.c_str(), &width, &height, &nChannels, 0);
        // auto ret = CreateTexture(textureTarget, data, width, height, nChannels);
        // stbi_image_free(data);
        // return ret;
        auto pic_data = stb_pic_data::create_stb_pic_data(dataFile);
        auto ret = CreateTexture(textureTarget, pic_data->data, pic_data->w, pic_data->h, pic_data->n);
        return ret;
    }
    inline auto CreateTextureByData(GLuint textureTarget, GLenum innerFormat, GLenum format, void *data, uint32_t width, uint32_t height)
    {

        GLuint texture;
        glActiveTexture(textureTarget); // binding textureTarget->GL_TEXTURE_2D
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture); // binding GL_TEXTURE_2D->texture, so there is textureTarget->texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        auto ret = TextureObject(texture, textureTarget);

        glTexImage2D(GL_TEXTURE_2D, 0, innerFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        ret.SetWHN(width, height, innerFormat <= GL_ALPHA ? 1 : (innerFormat - GL_RGB));
        return ret;
    };
} // namespace Helper

using glm::vec3;
/* note :
    1. mouseCallback scrollCallback processInput should add to glfw callback
    2. Tick should add to render loop begin
    3. use GetProjectionMatrix and GetViewMatrix to obtain relevant matrix
*/
class CameraWithController
{
public:
    CameraWithController(int sw, int sh) : screenWidth(sw), screenHeight(sh) {}
    int screenWidth, screenHeight;
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
    float constexpr static deltaTimeClipMax = 0.05;

    bool first = true; // record first time the follow work
    void mouseCallback(double xpos, double ypos)
    {
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
        auto cp = cos(glm::radians<float>(pitch));
        auto cy = cos(glm::radians<float>(yaw));
        glm::vec3 front;
        front.x = cos(glm::radians<float>(pitch)) * cos(glm::radians<float>(yaw));
        front.y = sin(glm::radians<float>(pitch));
        front.z = cos(glm::radians<float>(pitch)) * sin(glm::radians<float>(yaw));

        auto nf = glm::normalize(front);
        cameraFront = nf;
    }
    void scrollCallback(double offsetX, double offsetY)
    {
        if (fov >= 1.0 && fov <= 45)
            fov -= offsetY;
        fov = clamp(fov, 1, 45);
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
    void framebuffer_size_callback(GLFWwindow *window, int width_, int height_)
    {
        this->screenWidth = width_;
        this->screenHeight = height_;
    }
    auto GetProjectionMatrix()
    {
        return glm::perspective(glm::radians(fov), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
    }
    auto GetViewMatrix()
    {
        return glm::lookAt(cameraPos, cameraFront + cameraPos, cameraUp);
    }
    // when ct==0,this will disable keyboard affection
    void Tick(double currentTime)
    {
        deltaTime = glm::min<float>(deltaTimeClipMax, glm::max<float>(0, currentTime - lastFrameTime));
        lastFrameTime = currentTime;
    }
};
