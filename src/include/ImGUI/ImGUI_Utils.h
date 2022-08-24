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

using std::string;
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
    t.GetFlag();
    // t.GetMin();
    // t.GetMax();
    // t.GetSpeed();
    // t.GetFormat();
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

template <typename T>
struct Base_Type_Wrapper{
    std::string name;
    int flag;
    T data;
    Base_Type_Wrapper(const string&name,T data,int flag):name(name),flag(flag),data(data){}
    auto GetName() { return name; }
    auto GetFlag(){ return flag;}
};
// note:
// if you declare T as non-reference,then this will hold data as itself
// otherwise, the Universal_Type_Wrapper hold a reference to origin data
template <typename T>
 struct Universal_Type_Wrapper:public Base_Type_Wrapper<T>
 {
    Universal_Type_Wrapper(){
        static_assert(std::is_same_v<T,T>, "this imple is not allowed");
    }
 };
template <typename T>
requires( std::integral<std::remove_reference_t<T>>||
          std::floating_point<std::remove_reference_t<T>>)
 struct Universal_Type_Wrapper<T>:public Base_Type_Wrapper<T>
{
    using ValueType=T;
    using DT=std::remove_reference_t<ValueType>;
    static constexpr float int_speed=1.0f;
    static constexpr float float_speed=0.005;
    
    DT v_min,v_max;
    ImGuiSliderFlags_ flag;
    float speed;
    
    Universal_Type_Wrapper(const std::string &name,  T data, DT v_min=std::numeric_limits<T>::min(), DT v_max=std::numeric_limits<T>::max(),float speed=GetDefaultSpeed() ,ImGuiSliderFlags_ flag=ImGuiSliderFlags_AlwaysClamp)
     :Base_Type_Wrapper<T>(name,data,flag),v_min(v_min),v_max(v_max),speed(speed){};

   // dont need this
   // auto GetAttr() { return std::make_tuple(std::make_pair(name, &data)); }
    auto GetMin() { return v_min; }
    auto GetMax() { return v_max; }
    auto GetSpeed() { return speed;}
    auto GetFormat()
    {
        if constexpr (std::is_integral_v<DT>)
            return "%d";
        else if constexpr (std::is_floating_point_v<DT>)
            return "%.3ff";
       else if constexpr (std::is_same_v<T,T>)
           static_assert(!std::is_same_v<T,T>,"Unsupport Type");
    }

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
template <typename T>
requires(std::is_same_v<string,std::remove_reference_t<T>>)
 struct Universal_Type_Wrapper<T>:public Base_Type_Wrapper<T>
{
    using ValueType=T;
    using DT=std::remove_reference_t<ValueType>;
    Universal_Type_Wrapper(const string &name,T data,int flag=ImGuiInputTextFlags_AllowTabInput)
    :Base_Type_Wrapper<T>(name,data,flag){}
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
// show in checkbox
template <>
 struct Universal_Type_Wrapper<bool>:public Base_Type_Wrapper<bool>
{
    using ValueType=bool;
    using DT=std::remove_reference_t<ValueType>;
    Universal_Type_Wrapper(const string &name,bool data,int flag=ImGuiInputTextFlags_AllowTabInput)
    :Base_Type_Wrapper<bool>(name,data,flag){}

};
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
    // more customed attr
};
// clang-format on
template <Visible_Attr_Type T = Universal_Type_Wrapper<int>>
requires !Visible_Attr_Group_Type<T>
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
       // ImGui::SliderInt(t.GetName().c_str(), &data, t.GetMin(), t.GetMax(), t.GetFormat(), t.GetFlag());
     ImGui::DragInt(t.GetName().c_str(), &data, t.GetSpeed(), t.GetMin(), t.GetMax(), t.GetFormat(), t.GetFlag());
    }
    else if constexpr (std::is_same_v<RRT, bool>)
    {
        ImGui::Checkbox(t.GetName().c_str(),&t.data);
    }
    else if constexpr (std::is_same_v<RRT, std::string>)
    {
        int len=data.length(),temp_len=len*1.5+10;
        string temp;
        temp.reserve(temp_len);
        temp=data;
        auto name_hints=t.GetName()+":";
        ImGui::Text(name_hints.c_str());
        ImGui::InputTextMultiline(t.GetName().c_str(),const_cast<char*>(temp.data()),temp_len, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 2),t.GetFlag());
        data=temp.data();
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
inline void Draw_element(T &t,std::function<void()> beforeAttr=nullptr)
{
    static_assert(Qualified_Be_Wrapped<Example_Group_Type>,
                  "Example_Group_Type shoudl be qualified as Visible_Attr_Group_Type");

    auto &data = t.data;
    if (ImGui::CollapsingHeader(t.GetName().c_str(), t.GetFlag()))
    {
        if(beforeAttr!=nullptr){
            beforeAttr();
        }
        auto attrs = t.GetAllAttr();
        Draw_tuple_element(attrs);
        ImGui::Separator();
    }
}

