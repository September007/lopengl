#pragma once
#include <helper/OpenGL_Utils.h>
#include <helper/OpenGL_Objects.h>
#include <ImGUI/ImGUI_Utils.h>

using std::string;

// because Check_Render_Task_Completeness need imple the Qualified_Be_Wrapped constrain,
// for the sake of avoiding annoying template error, this may need to be the parent of
// Render_Task, to auto check if it's satisfy the constrain Qualified_Be_Wrapped.
template <typename T>
class Check_Render_Task_Completeness
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
    virtual bool prepareExecutingParameters() = 0;
    virtual void Execute() = 0;

    // forbid stack creating
    static void I_Render_Task_Deleter(I_Render_Task *p)
    {
        delete p;
    }
protected:
    virtual ~I_Render_Task() = 0;
};

class CentralController
{
public:
    std::vector<std::shared_ptr<I_Render_Task>> tasks;
    void Tick()
    {
        for (auto &task : tasks)
        {
            task->prepareExecutingParameters();
            task->Execute();
        }
    }
};

class NV12_to_RGB : public I_Render_Task, Check_Render_Task_Completeness<NV12_to_RGB>
{
    struct SettingParams{
        
    };

private:
    ~NV12_to_RGB(){};
};