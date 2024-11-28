#pragma once
#include <GLFW/glfw3.h>

#include "NodeWindow.h"

class Application
{
public:
    static Application* GetInstance() { return s_instance; }

    static Application* Create() { return s_instance = new Application(); }

    static void Destroy() { delete s_instance; }
    
    void Initialize();

    void Run();

    void Render();

    void Clean() const;

    static void Exit();

    static uint64_t GetFrameCount() { return s_instance->m_frameCount; }

    Vec2f GetWindowSize() const;

private:
    static Application* s_instance;
    
    GLFWwindow* m_window = nullptr;

    NodeWindow m_nodeWindow;

    uint64_t m_frameCount = 0;
    
};
