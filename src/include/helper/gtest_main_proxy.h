#pragma once
#include <functional>
#include <helper/error.h>

struct class_main_proxy
{
    std::function<int(int, char **)> main;
     std::string position;
    static auto &Instance()
    {
        static class_main_proxy ins={ std::function<int(int ,char **)>(nullptr), std::string("")};
        return ins;
    }

};

inline void main_proxy_register(std::function<int(int, char **)> main_, const string &position)
{

    if (class_main_proxy::Instance().main)
    {
        std::cerr << " there already registed one main_proxy from " << class_main_proxy::Instance().position << std::endl;
        exit(0);
    }
    class_main_proxy::Instance().main = main_;
    class_main_proxy::Instance().position = position;
}
#define REGISTER_MAIN_PROXY(main_) int ___wulala__ = (main_proxy_register(main_, fmt::format("{}:{}", __FILE__, __LINE__)), 0);