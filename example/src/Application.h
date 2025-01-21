#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "NodeWindow.h"

class Framebuffer;

class Application
{
public:
    static Application* GetInstance() { return s_instance; }

    static Application* Create() { return s_instance = new Application(); }

    static void Destroy() { delete s_instance; }
    
    void Initialize();
    
    static void DrawMainDock();
    void DrawMainBar() const;

    void WriteEditorFile(const std::string& path) const;
    void LoadEditorFile(const std::string& path) const;

    void Run();

    void Clean() const;

    Ref<Mesh> GetQuad() const;
    float GetTime() const {return m_time;}

    static void Exit();

    static uint64_t GetFrameCount() { return s_instance->m_frameCount; }

    Vec2f GetWindowSize() const;

    void UpdateShadersValues(int program) const;

private:
    static Application* s_instance;

    float m_time = 0;
    
    GLFWwindow* m_window = nullptr;

    NodeWindow* m_nodeWindow;

    uint64_t m_frameCount = 0;
    
};
