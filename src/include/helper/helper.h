#include <GLFW/glfw3.h>
#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <vector>
inline auto readFile(std::string f) -> std::string
{
    //  system("dir");
    std::ifstream fin(f);
    std::stringstream ss;
    ss << fin.rdbuf();
    auto ret = ss.str();
    return ret;
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
// \ret
// ret==0: success
// ret&1: checkAttributesExistence failed
// ret&2: checkPositionCorrectness failed
template <bool checkAttributesExistence = true, bool checkPositionCorrectness = true>
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

struct ShaderObject
{
    std::shared_ptr<GLint> shaderHandler = nullptr;
    //保存上一次获取的状态信息，构造函数初始化之后保存的是编译信息
    std::string  tempInfo;
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
        // 获取编译信息
        char infoLog[512];
        glGetShaderiv(*shaderHandler, GL_COMPILE_STATUS, &success);
        if (!success)
             tempInfo = infoLog;
    }
    auto Attach(GLint targetProgram) const -> void
    {
        return glAttachShader(targetProgram, *shaderHandler);
    }
    auto getInfo(GLenum pname){
        constexpr int tempLength=1024;
        char temp[tempLength];
        GLint params=GL_FALSE;
        glGetShaderiv(*shaderHandler,pname,&params);
        GLsizei len;
        glGetShaderInfoLog(*shaderHandler,tempLength,&len,temp);
        tempInfo={temp,temp+len};
        return params;
    }
    
};

using GLHandle = std::shared_ptr<GLint>;
struct ProgramObject
{
    GLHandle program = nullptr;
    //保存上一次获取的状态信息
    std::string  tempInfo;
    ProgramObject(GLint program = 0)
    {
        reset(program);
    }
    void reset(GLint program)
    {
        this->program.reset(new GLint(program), [](GLint *p)
                            { glDeleteProgram(*p); });
    }
    auto getProgram() { return *program; }
    auto getInfo(GLenum pname){
        constexpr int tempLength=1024;
        char temp[tempLength];
        GLint params=GL_FALSE;
        glGetProgramiv(getProgram(),pname,&params);
        GLsizei len;
        glGetProgramInfoLog(getProgram(),tempLength,&len,temp);
        tempInfo={temp,temp+len};
        return params;
    }
};

namespace
{
    template <bool isBegin, typename FirstShader, typename... RestShaders>
    GLint _CreateProgram(GLint program, FirstShader fs, RestShaders... rest)
    {
        static_assert(std::is_same_v<FirstShader, ShaderObject>, "只接受ShaderObject类型参数");

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