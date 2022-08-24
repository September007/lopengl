#pragma once
#include <helper/OpenGL_Utils.h>
#include <helper/OpenGL_Objects.h>
#include <ImGUI/ImGUI_Utils.h>

#include <fmt/format.h>
using std::string;
/*******************************************************************/
/************** automatocally set program uniforms by Wrapper ****/
/*******************************************************************/
inline void SetProgramParam(ProgramObject<true> &pro, Universal_Type_Wrapper<int> &i)
{
    pro.setInt(i.GetName(), i.data);
}

inline void SetProgramParam(ProgramObject<true> &pro, Universal_Type_Wrapper<float> &i)
{
    pro.setFloat(i.GetName(), i.data);
}

template <typename T, int index = 0>
inline void SetProgramParam(ProgramObject<true> &pro, Universal_Group_Wrapper<T> &i)
{
    using tupleT = decltype(Universal_Group_Wrapper<T>::GetAllAttr());
    if constexpr (std::tuple_size_v<tupleT> == index)
        return;
    else
    {
        auto attrs = i.GetAllAttr();
        SetProgramParam(pro, std::get<index>(attrs));
        SetProgramParam<T, index + 1>(pro, i);
    }
}

//          Generate simple VAO,VBO,VEO
template <int pos_dim = 3>
inline auto simpleV_ABE_O(float VL = -1, float VR = 1, float TCL = 0, float TCR = 1)
{
    // clang-format off
        float L=VL,R=VR;
        float TL=TCL,TR=TCR;
       // static float svs[24];
        float vertices3[]={
            L,L,0,TL,TL,
            L,R,0,TL,TR,
            R,R,0,TR,TR,
            R,L,0,TR,TL,
        };
        float vertices4[]={
            L,L,0,0,TL,TL,
            L,R,0,0,TL,TR,
            R,R,0,0,TR,TR,
            R,L,0,0,TR,TL,
        };
        //std::memcpy(svs,vertices,sizeof(svs)*sizeof(float));
        //static uint32_t sis[6];
        uint32_t indices[]={
            0,1,2,
            0,2,3,
        };
       // std::memcpy(sis,indices,sizeof(sis)*sizeof(uint32_t));
    // clang-format on
    auto vao = std::shared_ptr<Light::VertexArray>(Light::VertexArray::create());
    auto veo = std::shared_ptr<Light::IndexBuffer>(Light::IndexBuffer::create(indices, sizeof(indices)));
    if constexpr (pos_dim == 3)
    {
        auto vbo = std::shared_ptr<Light::VertexBuffer>(Light::VertexBuffer::create(vertices3, sizeof(vertices3)));
        return std::make_tuple(vao, vbo, veo);
    }
    else if constexpr (pos_dim == 4)
    {
        auto vbo = std::shared_ptr<Light::VertexBuffer>(Light::VertexBuffer::create(vertices4, sizeof(vertices4)));
        return std::make_tuple(vao, vbo, veo);
    }
    else if constexpr (pos_dim == pos_dim)
    {
        static_assert(pos_dim == pos_dim, "dim only support 3 or 4");
    }
}

// because Check_Render_Task_Completeness need imple the Qualified_Be_Wrapped constrain,
// for the sake of avoiding annoying template error, this may need to be the parent of
// Render_Task, to auto check if it's satisfy the constrain Qualified_Be_Wrapped.
template <typename T>
struct Check_Render_Task_Completeness
{
    Check_Render_Task_Completeness()
    {
        static_assert(Qualified_Be_Wrapped<T>, "I_Render_Shader implementation is not complete");
    }
};

class CentralController;
class I_Render_Task
{
public:
    string rsName;
    ProgramObject<true> program;
    CentralController *ccontroller = nullptr;
    I_Render_Task(string const &name, string const &vsSrc, string const &fsSrc, CentralController *cc)
        : rsName(name), ccontroller(cc),
          program(Helper::CreateProgram(ShaderObject(GL_VERTEX_SHADER, vsSrc),
                                        ShaderObject(GL_FRAGMENT_SHADER, fsSrc)))
    {
    }
    // beacuse we expose the setting params on the UI, so it need reload params every frame
    virtual bool PrepareExecutingParameters() = 0;
    virtual void Execute()
    {
        glUseProgram(program.getProgram());
        vao->bind();
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (GLuint *)0 + 3);
        GL_ERROR_STOP();
        vao->unbind();
        glUseProgram(0);
    };
    virtual void ShowConfig() = 0;
    // forbid stack creating
    static void I_Render_Task_Deleter(I_Render_Task *p)
    {
        delete p;
    }

    std::shared_ptr<Light::VertexArray> vao;
    std::shared_ptr<Light::VertexBuffer> vbo;
    std::shared_ptr<Light::IndexBuffer> veo;

protected:
    virtual ~I_Render_Task(){};
};

class CentralController
{
public:
    std::vector<std::shared_ptr<I_Render_Task>> tasks;
    void Tick()
    try
    {
        for (auto &task : tasks)
            try
            {
                task->PrepareExecutingParameters();
                task->Execute();
            }
            catch (std::exception &e)
            {
                std::cerr << fmt::format("catch error: {}", e.what()) << std::endl;
            }
        if (ImGui::Begin("hot config"))
        {
            for (auto &task : tasks)
                try
                {
                    task->ShowConfig();
                }
                catch (std::exception &e)
                {
                    std::cerr << fmt::format("catch error: {}", e.what()) << std::endl;
                }
        }
        ImGui::End();
    }
    catch (std::exception &e)
    {
        std::cerr << "catch exception: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "cant catch exception: " << std::endl;
    }
};

struct NV12_to_RGB : public I_Render_Task
{
    struct SettingParams : Check_Render_Task_Completeness<SettingParams>
    {
        struct Shader_Params
        {
            // could be set on outside
            Universal_Type_Wrapper<string> texture_path = {"texture", R"(F:/BMP/9.dib)"};
            Universal_Type_Wrapper<int> mode = {"mode", 0, 0, 3, 0.1};
            Universal_Type_Wrapper<int> dst_Width = {"dst_Width", 512, 256, 2048, 256};
            Universal_Type_Wrapper<int> dst_Height = {"dst_Height", 512, 256, 2048, 256};
            // generate by texture
            Universal_Type_Wrapper<int> overlay_Width = {"overlay_Width", 512, 256, 2048, 256};
            Universal_Type_Wrapper<int> overlay_Height = {"overlay_Height", 512, 256, 2048, 256};
            auto GetAllAttr() { return std::tie(texture_path, mode, dst_Width, dst_Height, overlay_Width, overlay_Height); }
        };
        Universal_Group_Wrapper<Shader_Params> shader_params = {"Shader params", {}};
        Universal_Type_Wrapper<bool> will_autogen_frame_wh = {"will autogen frame width and height", false};
        Universal_Type_Wrapper<int> frame_width = {"frame width", 1920, 256, 2048, 256};
        Universal_Type_Wrapper<int> frame_height = {"frame height", 1080, 256, 2048, 256};

        auto GetAllAttr() { return std::tie(shader_params, will_autogen_frame_wh, frame_width, frame_height); }
    };
    NV12_to_RGB(string const &name, string const &vsSrc, string const &fsSrc, CentralController *cc)
        : I_Render_Task(name, vsSrc, fsSrc, cc) {}
    Universal_Group_Wrapper<SettingParams> params = {"NV12_to_RGB", {}};

    bool PrepareExecutingParameters() override
    {
        resetOpenGLObjects();
        return true;
    }
    void Execute() override
    {
        glUseProgram(program.getProgram());
        vao->bind();
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (GLuint *)0 + 3);
        GL_ERROR_STOP();
        vao->unbind();
        glUseProgram(0);
    }
    void ShowConfig() override
    {
        // constexpr bool s=std::is_integral_v<bool>;
        Draw_element(params, []
                     {
            ImGui::Text("when Shader-params.dst_* and frame width is set up");
            ImGui::Text("the vertex coord will be generated automatically"); });
        if (params.data.will_autogen_frame_wh.data == true)
        {
            params.data.frame_height.data = params.data.shader_params.data.dst_Height.data;
            params.data.frame_width.data = params.data.shader_params.data.dst_Width.data;
        }
    }
    void resetOpenGLObjects()
    {
        // calc vertex position
        // xucl todo: use operator-> to simplify the longy reference like xxx.data.yyy to xxx->yyy
        float fh = params.data.frame_height.data, dh = params.data.shader_params.data.dst_Height.data;
        float fw = params.data.frame_width.data, dw = params.data.shader_params.data.dst_Width.data;
        auto vx = dw / fw * 2 - 1, vy = dh / fh * 2 - 1;
        std::tie(vao, vbo, veo) = simpleV_ABE_O<4>(vx, vy);

        Light::BufferLayout layout = {
            Light::BufferElement(Light::ShaderDataType::Float4, "position", false),
            Light::BufferElement(Light::ShaderDataType::Float2, "TextureUV", false)};
        vbo->setLayout(layout);
        vao->addVertexBuffer(vbo);
        vao->setIndexBuffer(veo);
    }

private:
    ~NV12_to_RGB(){

    };
};

struct Test_Render_Task : public I_Render_Task
{
    Test_Render_Task(string const &name, string const &vsSrc, string const &fsSrc, CentralController *cc)
        : I_Render_Task(name, vsSrc, fsSrc, cc) {}
    struct SettingParams
    {
        struct ShaderParams
        {
            Universal_Type_Wrapper<string> texturePath = {"aTexture", R"(F:/BMP/9.dib)"};
            Universal_Type_Wrapper<float> VL = {"vertex span-X", 1, -1, 1};
            Universal_Type_Wrapper<float> VR = {"vertex span-Y", 1, -1, 1};
            auto GetAllAttr() { return std::tie(texturePath, VL, VR); }
        };
        Universal_Group_Wrapper<ShaderParams> shader_params = {"shader params", {}};
        Universal_Type_Wrapper<string> vsSrc = {"vert shader source", R"(../media/shaders/quick_use_simple/this.vs.glsl)"};
        Universal_Type_Wrapper<string> fsSrc = {"frag shader source", R"(../media/shaders/quick_use_simple/this.fs.glsl)"};

        auto GetAllAttr() { return std::tie(shader_params, vsSrc, fsSrc); }
    };
    Universal_Group_Wrapper<SettingParams> params = {"Shader Setting", {}};
    // texture obj
    TextureObject tex = {-1, 0};
    bool PrepareExecutingParameters() override
    {
        program = Helper::CreateProgram(ShaderObject(GL_VERTEX_SHADER, readFile(params->vsSrc.data)),
                                        ShaderObject(GL_FRAGMENT_SHADER, readFile(params->fsSrc.data)));
        program.use();
        std::tie(vao, vbo, veo) = simpleV_ABE_O<3>();

        Light::BufferLayout layout = {
            Light::BufferElement(Light::ShaderDataType::Float3, "aPos", false),
            Light::BufferElement(Light::ShaderDataType::Float2, "aTexCoord", false)};
        tex = Helper::CreateTexture(GL_TEXTURE1, params->shader_params->texturePath.data);
        program.prepareVBO(*vbo.get());
        glClearColor(0.2, 0.2, 0.0, 1);
        program.setInt(params->shader_params->texturePath.GetName(), tex.targetTexture - GL_TEXTURE0);

        vbo->setLayout(layout);
        vao->addVertexBuffer(vbo);
        vao->setIndexBuffer(veo);
        program.unuse();
        return true;
    }
    void ShowConfig()
    {
        Draw_element(params, [] {

        });
    }
};