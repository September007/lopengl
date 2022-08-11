#include <glad/glad.h>
#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STRICT_ true

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

inline std::string getShaderInfoLog(GLint program, GLint shader)
{

    int ret_length;
    GLchar infol[1024];
    glGetShaderInfoLog(shader, 1024, &ret_length, infol);
    return infol;
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
            throw std::runtime_error("gl get error"); \
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
inline auto to_string(glm::vec<dim, typename T, Q> const &v)
{
    std::string ret;
    if constexpr (dim > 0)
        ret += std::to_string(v.x)+' ';
    if constexpr (dim > 1)
        ret += std::to_string(v.y)+' ';
    if constexpr (dim > 2)
        ret += std::to_string(v.z)+' ';
    if constexpr (dim > 3)
        ret += std::to_string(v.w)+' ';
    return ret;
}