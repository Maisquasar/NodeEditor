#pragma once
#include <GLFW/glfw3.h>

#include "MainWindow.h"

class Application
{
public:
    void Initialize();

    void Run();

    void Delete();

private:
    GLFWwindow* m_window = nullptr;

    MainWindow m_mainWindow;
    
};
