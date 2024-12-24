#pragma once
#include <functional>
#include <cstdint>
namespace Utils
{
    
    template<typename... Args>
    class Event {
    public:
        using Callback = std::function<void(Args...)>;
        Event() = default;
        Event& operator=(const Event& other) = default;
        Event(const Event&) = default;
        Event(Event&&) noexcept = default;
        virtual ~Event() = default;

        virtual void Bind(Callback callback)
        {
            m_callbacks.push_back(callback);
        }

        virtual void Invoke(Args... args)
        {
            for (auto& callback : m_callbacks)
            {
                callback(args...);
            }
        }

        virtual void Clear()
        {
            m_callbacks.clear();
        }
    private:
        std::vector<Callback> m_callbacks;
    };

    template<typename... Args>
    class EventWithID : public Event<Args...>
    {
    public:
        using Callback = std::function<void(Args...)>;
        EventWithID() = default;
        EventWithID& operator=(const EventWithID& other) = default;
        EventWithID(const EventWithID&) = default;
        EventWithID(EventWithID&&) noexcept = default;
        virtual ~EventWithID() = default;

        void Bind(Callback callback) override
        {
            m_callbacks[m_callbacks.size()] = callback;
        }

        void Bind(uint32_t id, Callback callback)
        {
            m_callbacks[id] = callback;
        }
        
        void Invoke(Args... args) override
        {
            for (auto& callback : m_callbacks)
            {
                callback.second(args...);
            }
        }

        void Remove(uint32_t id)
        {
            m_callbacks.erase(id);
        }
        
        void Clear() override
        {
            m_callbacks.clear();
        }
    private:
        std::unordered_map<uint32_t, Callback> m_callbacks;
    };
}