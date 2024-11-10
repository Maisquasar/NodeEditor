#pragma once
#include <GLFW/glfw3.h>

#include "MainWindow.h"

class Application
{
public:
    static Application* GetInstance() { return s_instance; }

    static Application* Create() { return s_instance = new Application(); }

    static void Destroy() { delete s_instance; }
    
    
    void Initialize();

    void Run();

    void Clean() const;

    static void Exit();

private:
    static Application* s_instance;
    
    GLFWwindow* m_window = nullptr;

    MainWindow m_mainWindow;
    
};
