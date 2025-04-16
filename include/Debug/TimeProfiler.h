#pragma once
#include <chrono>
#include <string>

template <typename... Args>
static std::string FormatString(const char* format, Args... args)
{
    int size = std::snprintf(nullptr, 0, format, args...);
    if (size < 0) return "";
    std::string buffer(size + 1, '\0');
    std::snprintf(&buffer[0], buffer.size(), format, args...);
    buffer.resize(size); // Remove null terminator
    return buffer;
}

class TimeProfiler
{
public:
    explicit TimeProfiler(const char* funcName)
    {
        m_startTime = std::chrono::steady_clock::now();
    }

        
    template <typename ...Args>
    TimeProfiler(const char* funcName, const char* format, Args ... args)
    {
        m_funcName = funcName;
        m_content = FormatString(format, args...);
        m_startTime = std::chrono::steady_clock::now();
    }
        
    ~TimeProfiler()
    {
        if (!m_funcName.empty())
            std::cout << FormatString("Function %s took %f ms : %s\n", m_funcName.c_str(), std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - m_startTime).count(), m_content.c_str()) << std::endl;
        else
            std::cout << FormatString("Function %s took %f ms\n", m_funcName.c_str(), std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - m_startTime).count()) << std::endl;
    }
private:
    std::chrono::time_point<std::chrono::steady_clock> m_startTime;
    std::string m_funcName;
    std::string m_content;
};

#define PROFILE_SCOPE() TimeProfiler funcNameProfiler(__FUNCTION__);
#define PROFILE_SCOPE_LOG(x, ...) TimeProfiler funcNameProfiler(__FUNCTION__, x, ##__VA_ARGS__);