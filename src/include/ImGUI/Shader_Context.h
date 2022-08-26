#pragma once
#include <helper/OpenGL_Utils.h>
#include <helper/OpenGL_Objects.h>
#include <ImGUI/ImGUI_Utils.h>
#include <helper/error.h>
#include <map>
#include <fmt/format.h>
using std::string;

template <typename T>
using Cache_Type_Wrapper = CachingWrapper<Universal_Type_Wrapper<T>>;
template <typename T>
using Cache_Group_Wrapper = CachingWrapper<Universal_Group_Wrapper<T>>;
/*******************************************************************/
/************** automatocally set program uniforms by Wrapper ****/
/*******************************************************************/
inline void checkExist(ProgramObject<true> &pro, const string &name)
{
    auto p = pro.getUniforms();
    auto ex = (p.find(name) != p.end());
    if (!ex)
    {
        std::cerr << fmt::format("uniform {} not found\n", name) << std::endl;
    }
}
inline void SetProgramParam(ProgramObject<true> &pro, Universal_Type_Wrapper<int> &i)
{
    checkExist(pro, i.GetName());
    pro.setInt(i.GetName(), i.data);
}

inline void SetProgramParam(ProgramObject<true> &pro, Universal_Type_Wrapper<float> &i)
{
    checkExist(pro, i.GetName());
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
inline auto detailed_simpleV_ABE_O(float HL = -1, float HR = 1, float VL = -1, float VR = 1,
                                   float THL = 0, float THR = 1, float TVL = 0, float TVR = 1)
{
    // clang-format off
	float vertices3[] = {
		HL,VL,0,THL,TVL,
		HL,VR,0,THL,TVR,
		HR,VR,0,THR,TVR,
		HR,VL,0,THR,TVL,
	};
	float vertices4[] = {
		HL,VL,0,0,THL,TVL,
		HL,VR,0,0,THL,TVR,
		HR,VR,0,0,THR,TVR,
		HR,VL,0,0,THR,TVL,
	};
	//std::memcpy(svs,vertices,sizeof(svs)*sizeof(float));
	//static uint32_t sis[6];
	uint32_t indices[] = {
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
template <int pos_dim = 3>
inline auto simpleV_ABE_O(float VL = -1, float VR = 1, float TCL = 0, float TCR = 1)
{
    return detailed_simpleV_ABE_O<pos_dim>(VL, VR, VL, VR, TCL, TCR, TCL, TCR);
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
        : rsName(name), ccontroller(cc), program(0)
    {
        try
        {
            program = Helper::CreateProgram(ShaderObject(GL_VERTEX_SHADER, vsSrc),
                                            ShaderObject(GL_FRAGMENT_SHADER, fsSrc));
        }
        catch (std::exception &e)
        {
            std::cerr << fmt::format("catch error {}", e.what());
        }
    }
    auto &GetName() { return rsName; }
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
    virtual std::string &GetVsSrcFile()=0;
    virtual std::string &GetFsSrcFile()=0;
    std::shared_ptr<Light::VertexArray> vao;
    std::shared_ptr<Light::VertexBuffer> vbo;
    std::shared_ptr<Light::IndexBuffer> veo;

    // use for in-app editoring shader source
   // Universal_Type_Wrapper<string > ss={"fs", "", ImGuiInputTextFlags_AllowTabInput,40};
    Cache_Type_Wrapper<string> fsSrcContent = {"fs", "", ImGuiInputTextFlags_AllowTabInput,40};
    CachingWrapper<bool> fsSrcShowing = false;
    CachingWrapper<string> fsSrcFilePath = {};
    void ResetShowingSrc(bool showHa, const string &fsSrcFile)
    {
        auto chgShow = !fsSrcShowing.Sync(showHa);
        auto oldPath = fsSrcFilePath.GetData();
        auto ChgSrc = !fsSrcFilePath.Sync(fsSrcFile);
        auto chgContent=fsSrcContent.SyncCache();
        // showing and  content change
        if(showHa&&!chgShow&&chgContent){
            writeFile(fsSrcFilePath.GetData(), fsSrcContent.GetData().data);
        }
        // with no change happening, just return
        if (!chgShow && !ChgSrc)
            return;
        // change show to not show
        if (fsSrcShowing == false && chgShow)
        {
            writeFile(fsSrcFilePath.GetData(), fsSrcContent.GetData().data);
            fsSrcFilePath = "";
            fsSrcContent.Sync({"fs", "", ImGuiInputTextFlags_AllowTabInput, fsSrcContent.GetLineCount()});
        }
        else if (fsSrcShowing == true && chgShow)
        {
            // change not show to show
            auto p = readFile(fsSrcFilePath.GetData());
            fsSrcContent.GetData().data = std::move(p);
            fsSrcContent.SyncCache();
        }
        else if (ChgSrc)
        {
            writeFile(oldPath, fsSrcContent.GetData().data);
            auto p = readFile(fsSrcFilePath.GetData());
            fsSrcContent.GetData().data = std::move(p);
            fsSrcContent.SyncCache();
        }
    }
    void ShowSrc(bool showHa, const string &fsSrcFile)
    {
        ResetShowingSrc(showHa, fsSrcFile);
        if (!showHa)
            return;
        if (ImGui::Begin(fmt::format("{} fs",GetName()).c_str()))
        {
            Draw_element(fsSrcContent);
        }
        ImGui::End();
    }

protected:
    virtual ~I_Render_Task(){};
};

class CentralController
{
public:
    // task name :  task itself
    std::map<std::string, std::shared_ptr<I_Render_Task>> tasks;
    std::string currentTaskName;
    Universal_Type_Wrapper<Type_Combo> which_shader = {"which shader", {}};
    static constexpr std::string_view currentTaskNameForAll = "all the tasks";

    // options to set modes like
    struct CC_Options : public Check_Render_Task_Completeness<CC_Options>
    {
        Universal_Type_Wrapper<Type_Combo> shader_refresh_mode = {"refresh", Type_Combo{{"each frame", "when change", "every custom length"}}};
        Universal_Type_Wrapper<bool> show_editor = {"show editor", false};
        Universal_Type_Wrapper<bool> show_compile_output = {"show compile", true};
        auto GetAllAttr() const { return std::tie(shader_refresh_mode, show_editor, show_compile_output); }
    };
    struct CC_Editor_Options : public Check_Render_Task_Completeness<CC_Editor_Options>
    {
        auto GetAllAttr() const { return std::tie(); }
    };
    struct CC_Compiler_Outputs_Options : public Check_Render_Task_Completeness<CC_Editor_Options>
    {
        auto GetAllAttr() const { return std::tie(); }
    };
    Cache_Group_Wrapper<CC_Options> cc_options = {"common option", CC_Options{}};
    Cache_Group_Wrapper<CC_Editor_Options> cc_editor_options = {"editor option", CC_Editor_Options{}};
    Cache_Group_Wrapper<CC_Compiler_Outputs_Options> cc_compiler_outputs_options = {"compiler output option", CC_Compiler_Outputs_Options{}};
    void Tick()
    try
    {
        if (ImGui::Begin("hot config"))
        {
            if (ImGui::BeginTabBar("Tab Options"))
            {
                // 1. shader option
                ShowConfig();
                // 2. shader exexcute
                auto execute_one = [](std::shared_ptr<I_Render_Task> &task)
                {
                    try
                    {
                        task->PrepareExecutingParameters();
                        task->Execute();
                    }
                    catch (std::exception &e)
                    {
                        std::cerr << fmt::format("catch error: {}", e.what()) << std::endl;
                    }
                };
                // config will list all the tasks out, and a checkbox is for this selecting;
                // current time this only support choose one or all.
                _Current_Choosed_Task_Relevant(execute_one);
                // 3. compiler options
                if (ImGui::BeginTabItem("compile"))
                {
                    Draw_element(cc_options);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
        // show editor
        _Show_FS_Editor();
    }
    catch (std::exception &e)
    {
        std::cerr << "catch exception: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "cant catch exception: " << std::endl;
    }
    void ShowConfig()
    {
        if (ImGui::BeginTabItem("shader"))
        {
            Draw_element(which_shader);
            auto draw_task = [](std::shared_ptr<I_Render_Task> &task)
            {
                try
                {
                    task->ShowConfig();
                }
                catch (std::exception &e)
                {
                    std::cerr << fmt::format("catch error: {}", e.what()) << std::endl;
                }
            };
            _Current_Choosed_Task_Relevant(draw_task);
            ImGui::EndTabItem();
        }
    }
    auto AddTask(std::shared_ptr<I_Render_Task> &&task)
    {
        auto ret = tasks.emplace(task->GetName(), task);
        if (ret.second)
            which_shader->options.push_back(tasks[task->GetName()]->GetName().c_str());
    }

protected:
    void  _Current_Choosed_Task_Relevant(std::function<void(std::shared_ptr<I_Render_Task> &)> fn)
    {
        // obtain data from option, change currentTaskName only if it's useable
        auto op = which_shader->GetChoosedOption();
        if (op.first)
            currentTaskName = op.second;
        if (currentTaskName == currentTaskNameForAll)
            for (auto &[name, task] : tasks)
                fn(task);
        else
        {
            std::shared_ptr<I_Render_Task> cs;
            if (auto p = tasks.find(currentTaskName); p != tasks.end())
                cs=p->second;
            else if (tasks.size())
                cs=tasks.begin()->second;
            fn(cs);
        }
    }
    void _Show_FS_Editor(){
        auto chgOptions=!cc_options.isSameAsCache();
        if(!chgOptions)return;
        auto show_fs_editor=[this](std::shared_ptr<I_Render_Task> &t){
            t->ShowSrc(this->cc_options.data.show_editor.data,t->GetFsSrcFile());
        };
        _Current_Choosed_Task_Relevant(show_fs_editor);
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
            // generated by texture
            Universal_Type_Wrapper<int> overlay_Width = {"overlay_Width", 512, 256, 2048, 256};
            Universal_Type_Wrapper<int> overlay_Height = {"overlay_Height", 512, 256, 2048, 256};
            auto GetAllAttr() const { return std::tie(texture_path, mode, dst_Width, dst_Height, overlay_Width, overlay_Height); }
        };
        Universal_Group_Wrapper<Shader_Params> shader_params = {"Shader params", {}};
        Universal_Type_Wrapper<bool> will_autogen_frame_wh = {"will autogen frame width and height", false};
        Universal_Type_Wrapper<int> frame_width = {"frame width", 1920, 256, 2048, 256};
        Universal_Type_Wrapper<int> frame_height = {"frame height", 1080, 256, 2048, 256};
        Universal_Type_Wrapper<string> vsSrc = {"vert shader source", R"(../src/test_frame/glsl/HANDSOUT/nv12_t0_rgb/nv12_t0_rgb.vs.glsl)"};
        Universal_Type_Wrapper<string> fsSrc = {"frag shader source", R"(../src/test_frame/glsl/HANDSOUT/nv12_t0_rgb/nv12_t0_rgb.fs.glsl)"};

        auto GetAllAttr() const { return std::tie(shader_params, will_autogen_frame_wh, frame_width, frame_height, vsSrc, fsSrc); }
    };
    NV12_to_RGB(string const &name, string const &vsSrc, string const &fsSrc, CentralController *cc)
        : I_Render_Task(name, vsSrc, fsSrc, cc) {}

    Universal_Group_Wrapper<SettingParams> params = {"NV12_to_RGB", {}};
    // texture obj
    TextureObject tex = {-1, 0};

    bool PrepareExecutingParameters() override
    {
        program = Helper::CreateProgram(ShaderObject(GL_VERTEX_SHADER, readFile(params->vsSrc.data)),
                                        ShaderObject(GL_FRAGMENT_SHADER, readFile(params->fsSrc.data)));

        auto temp_use = program.temp_use();

        // calc vertex position
        // xucl todo: use operator-> to simplify the longy reference like xxx.data.yyy to xxx->yyy
        float fh = params->frame_height.data, dh = params->shader_params->dst_Height.data;
        float fw = params->frame_width.data, dw = params->shader_params->dst_Width.data;
        auto vx = dw / fw * 2 - 1, vy = dh / fh * 2 - 1;
        std::tie(vao, vbo, veo) = detailed_simpleV_ABE_O<4>(0, vx, -vy, 0);

        Light::BufferLayout layout = {
            Light::BufferElement(Light::ShaderDataType::Float4, "position", false),
            Light::BufferElement(Light::ShaderDataType::Float2, "TextureUV", false)};
        vbo->setLayout(layout);
        vao->addVertexBuffer(vbo);
        vao->setIndexBuffer(veo);
        // xucl error: if all the shaders binding the same GL_TEXTURE1, in the serial calling in cc.Tick()
        // there would be a overwriting behaviour on this GL_TEXTURE1
        // xucl todo: here generate yuv data by hand
        tex = Helper::CreateTexture(GL_TEXTURE1, params->shader_params->texture_path.data);
        // program.setInt(params->shader_params->)
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
        if (params->will_autogen_frame_wh.data == true)
        {
            params->frame_height.data = params->shader_params->dst_Height.data;
            params->frame_width.data = params->shader_params->dst_Width.data;
        }
    }
    std::string &GetVsSrcFile()override{ return params->vsSrc.data;}
    std::string &GetFsSrcFile()override{ return params->fsSrc.data;}
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
            Universal_Type_Wrapper<float> HL = {"vertex horizontal X", -1, -1, 1};
            Universal_Type_Wrapper<float> HR = {"vertex horizontal Y", 1, -1, 1};
            Universal_Type_Wrapper<float> VL = {"vertex vertical X", -1, -1, 1};
            Universal_Type_Wrapper<float> VR = {"vertex vertical Y",  1, -1, 1};
            auto GetAllAttr() const { return std::tie(texturePath, HL, HR, VL, VR); }
        };
        Universal_Group_Wrapper<ShaderParams> shader_params = {"shader params", {}};
        Universal_Type_Wrapper<string> vsSrc = {"vert shader source", R"(../media/shaders/quick_use_simple/this.vs.glsl)"};
        Universal_Type_Wrapper<string> fsSrc = {"frag shader source", R"(../media/shaders/quick_use_simple/this.fs.glsl)"};

        auto GetAllAttr() const { return std::tie(shader_params, vsSrc, fsSrc); }
    };
    Universal_Group_Wrapper<SettingParams> params = {"Shader Setting", {}};
    // texture obj
    TextureObject tex = {-1, 0};
    bool PrepareExecutingParameters() override
    {
        program = Helper::CreateProgram(ShaderObject(GL_VERTEX_SHADER, readFile(params->vsSrc.data)),
                                        ShaderObject(GL_FRAGMENT_SHADER, readFile(params->fsSrc.data)));

        auto temp_use = program.temp_use();
        std::tie(vao, vbo, veo) = detailed_simpleV_ABE_O<3>(
            params->shader_params->HL.data, params->shader_params->HR.data,
            params->shader_params->VL.data, params->shader_params->VR.data);

        Light::BufferLayout layout = {
            Light::BufferElement(Light::ShaderDataType::Float3, "aPos", false),
            Light::BufferElement(Light::ShaderDataType::Float2, "aTexCoord", false)};
        tex = Helper::CreateTexture(GL_TEXTURE1, params->shader_params->texturePath.data);
        program.prepareVBO(*vbo.get());
        // glClearColor(0.2, 0.2, 0.0, 1);
        program.setInt(params->shader_params->texturePath.GetName(), tex.targetTexture - GL_TEXTURE0);

        vbo->setLayout(layout);
        vao->addVertexBuffer(vbo);
        vao->setIndexBuffer(veo);
        return true;
    }
    void ShowConfig()
    {
        Draw_element(params, [] {

        });
    }
    std::string &GetVsSrcFile()override{ return params->vsSrc.data;}
    std::string &GetFsSrcFile()override{ return params->fsSrc.data;}
};