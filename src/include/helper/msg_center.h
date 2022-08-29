#pragma once
#include <functional>
#include <map>
#include <mutex>

/*********************************************************************************/
/**************************                      *********************************/
/*********************************************************************************/
struct ScopeObject
{
    std::function<void()> cons;
    std::function<void()> decons;
    ScopeObject(
        std::function<void()> cons_, std::function<void()> decons_) 
        : cons{std::move(cons_)},decons{std::move(decons_)}
    {
        if(cons!=nullptr)
        cons();
    }
    ~ScopeObject()
    {
        if(decons!=nullptr)
        decons();
    }
};
struct lockable
{
    std::mutex m_;
    [[nodiscard]] auto temp_lock() { return std::unique_lock(m_); }
};

template<typename T>
using funcType=std::function<void()>;
template <typename msg_key>
class msg_center : public lockable
{
public:
    using MsgKey = std::decay_t<msg_key>;
    std::multimap<MsgKey, std::function<void()>> key_tasks;
    auto Register(MsgKey key, std::function<void()> f)
    {
        auto lg = temp_lock();
        return key_tasks.emplace(key, f);
    }
    auto search(MsgKey key)
    {
        temp_lock();
        std::set<std::function<void()>> ret;
        auto p = key_tasks.find(key);
        while (p != key_tasks)
        {
            ret.insert(p->second);
            ++p;
        }
        return ret;
    }
    auto call(MsgKey key)
    {
        auto lk = temp_lock();
        if (auto p = key_tasks.find(key); p != key_tasks.end())
        {
            while (p != key_tasks.end())
            {
                if (p->second != nullptr)
                    p->second();
                ++p;
            }
        }
    }
    auto Erase(MsgKey key)
    {
        auto lk = temp_lock();
        return key_tasks.erase(key);
    }
    static auto &Instance()
    {
        static msg_center mc;
        return mc;
    }

private:
    msg_center() {}
};

namespace Helper
{
    // it doesn't matter if type KT is reference or const or volatile or not
    template <typename KT>
    inline void Invoke(KT k)
    {
        msg_center<std::remove_cvref_t<KT>>::Instance().call(k);
    }
    template <typename KT>
    inline auto Search(KT k)
    {
        return msg_center<std::remove_cvref_t<KT>>::Instance().Search(k);
    }
    template <typename KT>
    inline void Erase(KT k)
    {
        msg_center<std::remove_cvref_t<KT>>::Instance().Erase(k);
    }
    template <typename KT>
    inline auto Register(KT k, std::function<void()> &&f)
    {
        return msg_center<std::remove_cvref_t<KT>>::Instance().Register(k, std::move(f));
    }
}
