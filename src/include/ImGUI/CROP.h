#pragma once
#include <ImGUI/Shader_Context.h>

struct CROP : public I_Render_Task
{
    struct SettingParams : Check_Render_Task_Completeness<SettingParams>
    {
        struct Shader_Params
        {
            // could be set on outside
            Universal_Type_Wrapper<string> texture_path = {"overlay", R"(F:/BMP/9.dib)"};
            Universal_Type_Wrapper<float> roi_x = {"roi_x", 1,-1,1 };
            Universal_Type_Wrapper<float> roi_y = {"roi_y", 0,-1,1 };
            Universal_Type_Wrapper<int> dst_Width = {"dst_Width", 512, 256, 2048, 256};
            Universal_Type_Wrapper<int> dst_Height = {"dst_Height", 512, 256, 2048, 256};
            // generated by texture
            Universal_Type_Wrapper<int> overlay_Width = {"overlay_Width", 512, 256, 2048, 256};
            Universal_Type_Wrapper<int> overlay_Height = {"overlay_Height", 512, 256, 2048, 256};
            auto GetAllAttr() const { return std::tie(texture_path, roi_x, roi_y, dst_Width, dst_Height, overlay_Width, overlay_Height); }
        };
        Universal_Group_Wrapper<Shader_Params> shader_params = {"Shader params", {}};
        Universal_Type_Wrapper<bool> will_autogen_frame_wh = {"will autogen frame width and height", false};
        Universal_Type_Wrapper<int> frame_width = {"frame width", 1920, 256, 2048, 256};
        Universal_Type_Wrapper<int> frame_height = {"frame height", 1080, 256, 2048, 256};
        Universal_Type_Wrapper<string> vsSrc = {"vert shader source", R"(../src/test_frame/glsl/HANDSOUT/crop/crop.vs.glsl)"};
        Universal_Type_Wrapper<string> fsSrc = {"frag shader source", R"(../src/test_frame/glsl/HANDSOUT/crop/crop.fs.glsl)"};
        auto GetAllAttr() const { return std::tie(shader_params, will_autogen_frame_wh, frame_width, frame_height, vsSrc, fsSrc); }
    };
    CROP(string const &name, string const &vsSrc, string const &fsSrc, CentralController *cc)
        : I_Render_Task(name, vsSrc, fsSrc, cc) {}

    Cache_Group_Wrapper<SettingParams> params = {"CROP", SettingParams{}};
    // texture obj
    TextureObject tex = {-1, 0};

    CachingWrapper<string> vsSrcContent;
    CachingWrapper<string> fsSrcContent;
    bool PrepareExecutingParameters(bool force_reset=false) override
    {
        auto chgParams=!params.SyncCache();
        vsSrcContent.SetSelf(readFile(params->vsSrc.data));
        fsSrcContent.SetSelf(readFile(params->fsSrc.data));
        // params and shader source not changing, just return
        if (program.getProgram()!=0&&!chgParams&&vsSrcContent.SyncCache()&&fsSrcContent.SyncCache())
            return true;
        program = Helper::CreateProgram(ShaderObject(GL_VERTEX_SHADER,vsSrcContent ),
                                        ShaderObject(GL_FRAGMENT_SHADER, fsSrcContent));
        auto temp_use = program.temp_use();
        // calc vertex position
        float fh = params->frame_height.data, dh = params->shader_params->dst_Height.data;
        float fw = params->frame_width.data, dw = params->shader_params->dst_Width.data;
        auto vx = dw / fw * 2, vy = dh / fh * 2;
        std::tie(vao, vbo, veo) = detailed_simpleV_ABE_O<4>(-1 + vx, -1, 1 - vy, 1);

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
        checkExist(program, params->shader_params->texture_path.GetName());
        SetProgramParam(program, params->shader_params->roi_x);
        SetProgramParam(program, params->shader_params->roi_y);
        SetProgramParam(program, params->shader_params->overlay_Height);
        SetProgramParam(program, params->shader_params->overlay_Width);

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
            params->frame_height.data = params->shader_params->dst_Height.data;
            params->frame_width.data = params->shader_params->dst_Width.data;
        }
    }
    std::string &GetVsSrcFile() override { return params->vsSrc.data; }
    std::string &GetFsSrcFile() override { return params->fsSrc.data; }

private:
    ~CROP(){

    };
};
