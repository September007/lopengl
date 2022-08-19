#pragma once
#include <glad/glad.h>
#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <helper/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>
#include <type_traits>
#define STRICT_ true

#include <iostream>
#define report_error(msg) do{std::cerr<<"at "<<__FILE__<<":"\
        <<__LINE__<<std::endl<<(msg)<<std::endl;\
        throw std::runtime_error(msg);}while(0)

template <bool strict_ = STRICT_>
inline auto readFile(const std::string &f) -> std::string
{
    try
    {
        //  system("dir");
        std::ifstream fin;
        fin.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fin.open(f);
        std::stringstream ss;
        ss << fin.rdbuf();
        auto ret = ss.str();
        return ret;
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
        if constexpr (strict_)
            throw;
        else
            return "";
    }
}

inline auto writeFile(const std::string &fName, const std::string &data)
{
    std::ofstream out(fName);
    out << data;
    out.close();
    return out.good();
}

using AttributeInfo = struct
{
    std::string name;
    GLenum type;
    GLint position;
    GLint size;
};

// use after program link
inline auto getAttributes(GLint program)
{
    std::map<std::string, AttributeInfo> ret;
    int count = 0;
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &count);
    constexpr int name_length = 256;
    char cache_name[name_length];
    GLint used_cache_name_length = 0;
    for (int i = 0; i < count; ++i)
    {
        AttributeInfo info;
        glGetActiveAttrib(program, GLint(i), name_length, &used_cache_name_length, &info.size, &info.type, cache_name);
        info.name = std::string(cache_name, cache_name + used_cache_name_length);
        info.position = glGetAttribLocation(program, info.name.c_str());
        ret[info.name] = info;
    }
    return ret;
}

inline int error;
#define GL_ERROR_STOP()                               \
    {                                                 \
        error = glGetError();                         \
        if (error != 0)                               \
            report_error("gl get error"); \
    }
// \ret
// ret==0: success
// ret&1: checkAttributesExistence failed
// ret&2: checkPositionCorrectness failed
template <bool checkAttributesExistence = STRICT_, bool checkPositionCorrectness = STRICT_>
inline int bindAttributesLocations(GLint program, std::vector<std::pair<std::string, GLint>> const &lcos)
{
    int ret = 0;
    if constexpr (checkAttributesExistence)
        for (auto &[name, _] : lcos)
            if (-1 == glGetAttribLocation(program, name.c_str()))
            {
                ret += 1;
                break;
            }
    std::set<int> s;
    for (auto &[name, pos] : lcos)
    {
        glBindAttribLocation(program, pos, name.c_str());
        s.insert(pos);
    }
    // simply check size
    if constexpr (checkPositionCorrectness)
        if (int(s.size()) < int(lcos.size()))
            ret += 2;
    glLinkProgram(program);
    return ret;
}

template <bool strict_ = STRICT_>
struct ShaderObject
{
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
        // check program link status
        auto ret = getInfo(GL_LINK_STATUS);
        if constexpr (strict_)
            if (ret != GL_TRUE)
                throw std::runtime_error("program link failed:\n\t" + tempInfo);
    }
    GLint getProgram() const { return *program; }
    auto getInfo(GLenum pname)
    {
        constexpr int tempLength = 1024;
        char temp[tempLength];
        GLint params = GL_FALSE;
        glGetProgramiv(getProgram(), pname, &params);
        GLsizei len;
        glGetProgramInfoLog(getProgram(), tempLength, &len, temp);
        tempInfo = {temp, temp + len};
        return params;
    }
    auto getAttributes()
    {
        return ::getAttributes(getProgram());
    }
    auto getUniforms()
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
        glUniform1i(glGetUniformLocation(getProgram(), name.c_str()), (int)value);
    }
    void setInt(const std::string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(getProgram(), name.c_str()), value);
    }
    void setFloat(const std::string &name, float value) const
    {
        glUniform1f(glGetUniformLocation(getProgram(), name.c_str()), value);
    }
    int getUniformLocation(const std::string &name)
    {
        return glGetUniformLocation(getProgram(), name.c_str());
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

template <int dim, typename T, glm::qualifier Q>
inline auto to_string(typename glm::vec<dim, T, Q> const &v)
{
    std::string ret;
    if constexpr (dim > 0)
        ret += std::to_string(v.x) + ' ';
    if constexpr (dim > 1)
        ret += std::to_string(v.y) + ' ';
    if constexpr (dim > 2)
        ret += std::to_string(v.z) + ' ';
    if constexpr (dim > 3)
        ret += std::to_string(v.w) + ' ';
    return ret;
}

// from left to right
// if these transform matrix let
template <typename... restMAT>
inline int ProjectionOutOfRange(glm::vec4 c)
{
    return c.x > 1 || c.x < -1 || c.y > 1 || c.y < -1 || c.z > 1 || c.z < -1;
}
template <typename... restMAT>
inline int ProjectionOutOfRange(glm::vec4 v, restMAT... rest)
{
    auto vv = (... * rest) * v;
    return ProjectionOutOfRange(vv);
}

// no postfix!!!
inline auto getSrcFileNameOnlyName(std::string const &fullName)
{
    std::filesystem::path path = fullName;
    auto filename = path.filename().string();
    auto dotPos = filename.find_last_of('.');
    return filename.substr(0, dotPos);
}

template <typename T, typename T1 = T, typename T2 = T>
inline auto clamp(T val, T1 minVal, T2 maxVal) -> T
{
    static_assert(std::is_convertible_v<T1, T> && std::is_convertible_v<T2, T>, "T1 or T2 is not convertable to T");
    if (val < minVal)
        return minVal;
    if (val > maxVal)
        return maxVal;
    return val;
}

template <typename UNIT, typename TO>
std::string toRGB(UNIT *data, int pixelLength)
{
    std::string ret;
    ret.resize(3 * pixelLength * sizeof(TO));
    for (int i = 0; i < pixelLength; ++i)
    {
        void *p = ret.data() + sizeof(TO) * i * 3;
        TO *pp = reinterpret_cast<TO *>(p);
        pp[0] = data[i * 3];
        pp[1] = data[i * 3 + 1];
        pp[2] = data[i * 3 + 2];
    }
    return ret;
}

class TextureObject
{
public:
    std::shared_ptr<GLuint> obj;
    int width, height, nChannels;
    TextureObject() = delete;
    TextureObject(GLuint obj_)
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
};
namespace Helper
{
    inline auto CreateTexture(GLuint textureTarget, void *data, int width, int height, int nChannels)
    {
        GLuint texture;
        glActiveTexture(textureTarget);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        auto ret = TextureObject(texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB + nChannels - 3, width, height, 0, GL_RGB + nChannels - 3, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        ret.SetWHN(width, height, nChannels);
        return ret;
    }
    inline auto CreateTexture(GLuint textureTarget, const std::string &dataFile)
    {
        int width, height, nChannels;
        auto data = stbi_load(dataFile.c_str(), &width, &height, &nChannels, 0);
        auto ret = CreateTexture(textureTarget, data, width, height, nChannels);
        stbi_image_free(data);
        return ret;
    }
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
    this->screenWidth=width_;
    this->screenHeight=height_;
}
    auto GetProjectionMatrix()
    {
        return glm::perspective(glm::radians(fov), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
    }
    auto GetViewMatrix(){
        return glm::lookAt(cameraPos, cameraFront + cameraPos, cameraUp);
    }
    // when ct==0,this will disable keyboard affection
    void Tick(double currentTime){
        deltaTime = glm::min<float>(deltaTimeClipMax,glm::max<float>(0, currentTime - lastFrameTime));
        lastFrameTime = currentTime;
    }
};