#pragma once
#include <ImGUI/Shader_Context.h>

struct RGB2NV12 : public I_Render_Task
{
    struct SettingParams : Check_Render_Task_Completeness<SettingParams>
    {
        struct Shader_Params
        {
            // could be set on outside
            Universal_Type_Wrapper<string> texture_path = {"overlay", R"(../media/texture/bmp/9.dib)"};
            Universal_Type_Wrapper<int> overlay_Height = {"overlay_Height", 512, 256, 1080, 256};
            Universal_Type_Wrapper<int> overlay_Width = {"overlay_Width", 512, 256, 1920, 256};
            Universal_Type_Wrapper<int> dst_Height = {"dst_Height", 512, 256, 1080, 256};
            Universal_Type_Wrapper<int> dst_Width = {"dst_Width", 512, 256, 1920, 256};
            Universal_Type_Wrapper<int> mode = {"mode", 0, 0, 1, 0.01};
            auto GetAllAttr() const { return std::tie(texture_path, mode, overlay_Width, overlay_Height, dst_Width, dst_Width); }
        };
        Universal_Group_Wrapper<Shader_Params> shader_params = {"Shader params", {}};
        Universal_Type_Wrapper<bool> will_autogen_frame_wh = {"will autogen frame width and height", false};
        Universal_Type_Wrapper<int> frame_width = {"frame width", 1920, 256, 2048, 256};
        Universal_Type_Wrapper<int> frame_height = {"frame height", 1080, 256, 2048, 256};
        Universal_Type_Wrapper<string> vsSrc = {"vert shader source", R"(../src/test_frame/glsl/HANDSOUT/rgb_to_nv12/rgb_to_nv12.vs.glsl)"};
        Universal_Type_Wrapper<string> fsSrc = {"frag shader source", R"(../src/test_frame/glsl/HANDSOUT/rgb_to_nv12/rgb_to_nv12.fs.glsl)"};
        auto GetAllAttr() const { return std::tie(shader_params, will_autogen_frame_wh, frame_width, frame_height, vsSrc, fsSrc); }
    };
    RGB2NV12(string const &name, string const &vsSrc, string const &fsSrc, CentralController *cc)
        : I_Render_Task(name, vsSrc, fsSrc, cc) {}

    Cache_Group_Wrapper<SettingParams> params = {"RGB2NV12", SettingParams{}};
    // texture obj
    TextureObject tex = {std::numeric_limits<GLuint>::max(), 0};

    CachingWrapper<string> vsSrcContent;
    CachingWrapper<string> fsSrcContent;
    bool PrepareExecutingParameters(bool force_reset = false) override
    {
        auto chgParams = !params.SyncCache();
        vsSrcContent.SetSelf(readFile(params->vsSrc.data));
        fsSrcContent.SetSelf(readFile(params->fsSrc.data));
        if (chgParams)
            glfwSetWindowSize(Light::OpenGLContext::CurrentContext()->GetHandle(), params->frame_width.data, params->frame_height.data);
        // params and shader source not changing, just return
        if (program.getProgram() != 0 && !chgParams && vsSrcContent.SyncCache() && fsSrcContent.SyncCache())
            return true;
        program = Helper::CreateProgram(ShaderObject(GL_VERTEX_SHADER, vsSrcContent),
                                        ShaderObject(GL_FRAGMENT_SHADER, fsSrcContent));
        auto temp_use = program.temp_use();
        // calc vertex position
        std::tie(vao, vbo, veo) = detailed_simpleV_ABE_O<4>(-1, 1, -1, 1);

        // std::tie(vao, vbo, veo) = detailed_simpleV_ABE_O<4>(-1, 1, -1, 1);
        Light::BufferLayout layout = {
            Light::BufferElement(Light::ShaderDataType::Float4, "position", false),
            Light::BufferElement(Light::ShaderDataType::Float2, "TextureUV", false)};
        // xucl error: if all the shaders binding the same GL_TEXTURE1, in the serial calling in cc.Tick()
        // there would be a overwriting behaviour on this GL_TEXTURE1
        tex = Helper::CreateTexture(GL_TEXTURE1, params->shader_params->texture_path.data);
        program.prepareVBO(*vbo.get());
        program.setInt(params->shader_params->texture_path.GetName(), 1);

        vbo->setLayout(layout);
        vao->addVertexBuffer(vbo);
        vao->setIndexBuffer(veo);
        auto unis = program.getUniforms();
        auto attrs = program.getAttributes();
        checkExist(program, params->shader_params->texture_path.GetName());
        SetProgramParam(program, params->shader_params);

        return true;
    }
    void ShowConfig() override
    {
        using T = decltype(params);
        constexpr bool x[] = {
            Visible_Attr_Group_Type<T>,
            Visible_Attr_Type<std::add_const_t<decltype(params->frame_height)>>};
        const auto p = params->frame_height;
        // constexpr bool s=std::is_integral_v<bool>;
        Draw_element(params, []
                     {
            ImGui::Text("when Shader-params.dst_* and frame width is set up");
            ImGui::Text("the vertex coord could be generated automatically"); });
        if (params->will_autogen_frame_wh.data == true)
        {
            params->frame_height.data = params->shader_params->overlay_Height.data = params->shader_params->dst_Height.data = tex.height;
            params->frame_width.data = params->shader_params->overlay_Width.data = params->shader_params->dst_Width.data = tex.width;
        }
    }
    std::string &GetVsSrcFile() override { return params->vsSrc.data; }
    std::string &GetFsSrcFile() override { return params->fsSrc.data; }

private:
    ~RGB2NV12(){

    };
};