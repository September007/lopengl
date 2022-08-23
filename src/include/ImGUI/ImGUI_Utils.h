#pragma once
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <stdio.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// clang-format off
#include <concepts>
#include <type_traits>
#include <tuple>
#include <string>
template <typename... T>
struct is_from_tuple : public std::false_type{};
template <typename... T>
struct is_from_tuple<std::tuple<T...>> : public std::true_type{};
template <typename... T>
constexpr bool is_from_tuple_v = is_from_tuple<T...>::value;

// constraint for type to be use as attr to show
// show in DragInt or DragFloat
template <typename T>
concept Visible_Attr_Type = requires(T t)
{
    t.GetName();
    t.GetMin();
    t.GetMax();
    t.GetSpeed();
    t.GetFormat();
    t.GetFlag();
}
&&std::same_as<std::string, decltype(std::declval<T>().GetName())>;

// constraint for type to be use as attr group to show
// show in CollapsingHeader
template <typename T>
concept Visible_Attr_Group_Type=requires(T t){
    // reflection
    t.GetName();
    t.GetAllAttr();
};
template<typename T>
concept Qualified_Be_Wrapped=requires(T t){
    t.GetAllAttr();
};
// note:
// if you declare T as non-reference,then this will hold data as itself
// otherwise, the Universal_Type_Wrapper hold a reference to origin data
template <typename T>
requires( std::integral<std::remove_reference_t<T>>||
          std::floating_point<std::remove_reference_t<T>>)
 struct Universal_Type_Wrapper
{
    using ValueType=T;
    using DT=std::remove_reference_t<ValueType>;
    static constexpr float int_speed=0.0000001;
    static constexpr float float_speed=0.005;
    
    std::string name;
    T data;
    DT v_min,v_max;
    ImGuiSliderFlags_ flag;
    float speed;
    Universal_Type_Wrapper(const std::string &name,  T data, DT v_min=std::numeric_limits<T>::min(), DT v_max=std::numeric_limits<T>::max(),float speed=GetDefaultSpeed() ,ImGuiSliderFlags_ flag=ImGuiSliderFlags_AlwaysClamp)
     :name(name), data(data),v_min(v_min),v_max(v_max),flag(flag){};

    auto GetName() { return name; }
   // dont need this
   // auto GetAttr() { return std::make_tuple(std::make_pair(name, &data)); }
    auto GetMin() { return v_min; }
    auto GetMax() { return v_max; }
    auto GetSpeed() { return speed;}
    auto GetFormat()
    {
        if constexpr (std::is_integral_v<DT>)
            return "%lld";
        else if constexpr (std::is_floating_point_v<DT>)
            return "%lf";
       else if constexpr (std::is_same_v<T,T>)
           static_assert(!std::is_same_v<T,T>,"Unsupport Type");
    }
    auto GetFlag(){ return flag;}

   private:
    static auto constexpr GetDefaultSpeed(){
        if constexpr (std::is_integral_v<DT>)
            return int_speed;
        else if constexpr (std::is_floating_point_v<DT>)
            return float_speed;
       else if constexpr (std::is_same_v<T,T>)
           static_assert(!std::is_same_v<T,T>,"Unsupport Type");
    }
};
namespace {
    struct Example_Group_Type{
        Universal_Type_Wrapper<int> i;
        Universal_Type_Wrapper<float> f;
        auto GetAllAttr(){
            return std::tie(i,f);
        }
    };
}
// note if type T is qualified to be wrapped,
// it need implement GetAllAttr() ,see Example_Group_Type
template<typename T>
requires std::is_class_v<std::remove_reference_t<T>>
struct Universal_Group_Wrapper{
    using ValueType=T;
    T data;
    std::string name;
    ImGuiTreeNodeFlags_ flag;
    Universal_Group_Wrapper(const std::string&name,T data,
    ImGuiTreeNodeFlags_ flag=ImGuiTreeNodeFlags_(ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen|ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen)):
    name(name),data(data),flag(flag){}
    auto GetName() { return name; }
    auto GetFlag(){ return flag; }
    auto GetAllAttr(){
        return data.GetAllAttr();
    }
};
// clang-format on
template <Visible_Attr_Type T = Universal_Type_Wrapper<int>>
inline void Draw_element(T &t)
{
    using RRT = std::remove_reference_t<typename T::ValueType>;
    auto &data = t.data;
    if constexpr (std::is_same_v<RRT, float>)
    {
        ImGui::DragFloat(t.GetName().c_str(), &data, 0.005, t.GetMin(), t.GetMax(), t.GetFormat(), t.GetFlag());
    }
    else if constexpr (std::is_same_v<RRT, int>)
    {
        ImGui::DragInt(t.GetName().c_str(), &data, t.GetSpeed(), t.GetMin(), t.GetMax(), t.GetFormat(), t.GetFlag());
    }
    else if constexpr (std::is_same_v<T, T>)
    {
        static_assert(!std::is_same_v<T, T>, "Unsupport Type");
    }
};
namespace
{
    template <typename tupleT, int index = 0>
    inline void Draw_tuple_element(tupleT &attrs)
    {
        if constexpr (index == std::tuple_size_v<tupleT>)
            return;
        else
        {
            Draw_element(std::get<index>(attrs));
            Draw_tuple_element<tupleT, index + 1>(attrs);
        }
    }
}
template <Visible_Attr_Group_Type T>
inline void Draw_element(T &t)
{
    static_assert(Qualified_Be_Wrapped<Example_Group_Type>,
                  "Example_Group_Type shoudl be qualified as Visible_Attr_Group_Type");

    auto &data = t.data;
    if (ImGui::CollapsingHeader(t.GetName().c_str(), t.GetFlag()))
    {
        auto attrs = t.GetAllAttr();
        Draw_tuple_element(attrs);
    }
}

