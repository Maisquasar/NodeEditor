#include "Application.h"

#include <galaxymath/Maths.h>

#include "NodeSystem/NodeTemplateHandler.h"
#include "Render/Framebuffer.h"
using namespace GALAXY;
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>

Application* Application::s_instance = nullptr;
static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


static Mesh s_meshTest;
static Shader s_shaderTest;
void Application::Initialize()
{
    // Initialize GLFW
    if (!glfwInit()) {
        return;
    }

    std::cout << "GLFW version: " << glfwGetVersionString() << std::endl;

    // Create a windowed mode window and its OpenGL context
    m_window = glfwCreateWindow(1280, 720, "Hello ImGui", NULL, NULL);
    if (!m_window) {
        glfwTerminate();
        return;
    }

    // Maximize the window
    // glfwMaximizeWindow(m_window);

    // Make the window's context current
    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        return;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 130"); // Adjust version as needed

    uint64_t fromUUID = NodeTemplateHandler::TemplateIDFromString("1 - x");
    uint64_t toUUID = NodeTemplateHandler::TemplateIDFromString("One Minus");

    m_mainWindow.Initialize();
    if (!s_shaderTest.Load("shaders/defaultShader"))
    {
        std::cout << "Failed to load shader" << std::endl;
    }
    s_meshTest.Initialize(quadVertices.data(), 6);
}

void Application::Run()
{
    // Main loop
    while (!glfwWindowShouldClose(m_window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        m_mainWindow.Draw();

        m_mainWindow.Update();

        ImGui::Render();

        /*
        int display_w, display_h;
        glfwGetFramebufferSize(m_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        */
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        s_shaderTest.Use();
        s_meshTest.Draw();
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
        // Swap buffers
        glfwSwapBuffers(m_window);
        m_frameCount++;
    }
        
}

void Application::Render()
{
    
}

void Application::Clean() const
{
    m_mainWindow.Delete();
    
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Application::Exit()
{
    auto instance = GetInstance();
    glfwSetWindowShouldClose(instance->m_window, true);
}

Vec2f Application::GetWindowSize() const
{
    Vec2i windowSize;
    glfwGetWindowSize(m_window, &windowSize.x, &windowSize.y);
    return windowSize;
}
