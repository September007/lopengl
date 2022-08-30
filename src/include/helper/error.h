#pragma once
#include <fmt/format.h>
#include <string>
#include <map>
#include <set>
#include <chrono>
#include <iostream>
#include <mutex>
using std::string;

// not mature yet
#define distribute_macros3(l, m, r, ...) \
    l(__VA_ARGS__);                      \
    m(__VA_ARGS__);                      \
    r(__VA_ARGS__)
#define define_enums(enum_name, ...) enum class enum_name \
{                                                         \
    __VA__ARGS__                                          \
}
#define mapping_enums(enum_name, map_method, ...) \
    {                                             \
    }

namespace error_map
{

    using error_code = enum error_code {
        notify,
        info,
        warning,
        error,
        cnt_erro_code
    };
    using object_code = enum object_code {
        shader,
        vert_shader,
        frag_shader,
        shader_program,
        exe,
        cnt_object_code
    };
    using message_code = enum shader_error_code {
        compile,
        link,
        cnt_message_code
    };
    using container = int64_t;
    inline constexpr static std::string_view ec_msg[] = {
        "notify",
        "info",
        "warning",
        "error",
        "cnt_erro_code"};
    inline constexpr static std::string_view oc_msg[] = {
        "shader",
        "vert_shader",
        "frag_shader",
        "shader_program",
        "exe",
        "cnt_object_code"};
    inline constexpr static std::string_view mc_msg[] = {
        "compile",
        "link",
        "cnt_message_code"};
    template <error_code ec, object_code oc, message_code mc>
    auto msg_mapping()
    {
        constexpr static std::string error_desc = fmt::format("[{}] {}.{}:", ec_msg[ec], oc_msg[oc], mc_msg[mc]);
        return error_desc;
    }
    inline auto msg_mapping(error_code ec, object_code oc, message_code mc)
    {
        std::string error_desc = fmt::format("[{}] {}.{}:", ec_msg[ec], oc_msg[oc], mc_msg[mc]);
        return error_desc;
    }
    struct code_packet
    {
        error_code ec;
        object_code oc;
        message_code mc;
        code_packet(error_code ec, object_code oc, message_code mc) : ec(ec), oc(oc), mc(mc) {}
        // why if i doesn't declare default ,there wil be a error
        bool operator== (code_packet const&) const =default;
        bool operator!= (code_packet const&) const =default;
        bool operator< (code_packet const&) const =default;
        std::strong_ordering operator<=>(const code_packet&) const = default;
    };
    inline auto msg_mapping(code_packet cp)
    {
        return msg_mapping(cp.ec, cp.oc, cp.mc);
    }
    using clock = std::chrono::system_clock;
    struct desc_packet
    {
       // std::chrono::zoned_time<clock::duration> code_time = std::chrono::system_clock::now();
        clock::time_point code_time=clock::now();
        code_packet cp;
        std::string msg;
        // bool operator<(desc_packet const &ot)
        // {
        //     // if (code_time.get_sys_time() != ot.code_time.get_sys_time())
        //     //     return code_time.get_sys_time() < ot.code_time.get_sys_time();
        //     if (cp != ot.cp)
        //         return cp < ot.cp;
        //     return msg < ot.msg;
        // }
        bool operator== (desc_packet const&) const =default;
        bool operator!= (desc_packet const&) const =default;
        std::strong_ordering operator<=>(const desc_packet&) const = default;
        auto time_desc()
        {
            // check https://en.cppreference.com/w/cpp/chrono/zoned_time/formatter

// why there defined not work
#ifdef _MSVC_LANG
            return std::format(std::cin.getloc(), "%T", code_time);
#elif defined(__GNUC__)
            return "";
#elif defined(__clang__)
            return "";
#endif
        }
        auto get_desc() { return fmt::format("{}\n{}: {}", time_desc(), msg_mapping(cp), msg); }
    };
    // global error code recorder
    struct Code_Recorder
    {
        std::set<desc_packet> cps;
        std::mutex m_;
        [[nodiscard]] auto temp_lock() { return std::unique_lock(m_); }
        auto insert(desc_packet &&p)
        {
            auto lg = temp_lock();
            return cps.insert(p);
        }
        void clear()
        {
            auto lg = temp_lock();
            return cps.clear(); // may clear each time these registed file fresh
        }
        auto begin() { return cps.begin(); }
        auto end() { return cps.end(); }
        static auto &Instance()
        {
            static Code_Recorder ins;
            return ins;
        }

    private:
        Code_Recorder(){};
    };
    inline void report_error(error_code ec, object_code oc, message_code mc, const string &msg)
    {
        desc_packet p = {clock::now(), {ec, oc, mc}, msg};
        // do something before record it
        Code_Recorder::Instance().insert(std::move(p));
    }
}