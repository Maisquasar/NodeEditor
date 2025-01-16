#include "Application.h"

#include <filesystem>
#include <imgui_internal.h>
#include <galaxymath/Maths.h>
#include <cpp_serializer/CppSerializer.h>

#include "NodeEditor.h"
#include "NodeSystem/ShaderMaker.h"
#include "Render/Framebuffer.h"

using namespace GALAXY;
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include "Serializer.h"

#define EDITOR_FILE_NAME "editor.settings"
#define TEMP_FOLDER "tmp/"
#define SAVE_FOLDER "save/"



Application* Application::s_instance = nullptr;
static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void APIENTRY glDebugOutput(GLenum source, 
                            GLenum type, 
                            unsigned int id, 
                            GLenum severity, 
                            GLsizei length, 
                            const char *message, 
                            const void *userParam)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " <<  message << std::endl;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}

void Application::UpdateShadersValues(int program) const
{
    // Set time value
    GLint timeLocation = glGetUniformLocation(program, "Time");
    if (timeLocation != -1) {
        float time = GetTime();
        glUniform1f(timeLocation, time);
    }
}


void Application::Initialize()
{
    // Initialize GLFW
    if (!glfwInit()) {
        return;
    }

    std::filesystem::create_directory(TEMP_FOLDER);

    std::cout << "GLFW version: " << glfwGetVersionString() << std::endl;

    // Create a windowed mode window and its OpenGL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);  
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
    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    } 

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* shadingLangVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version supported: " << version << std::endl;
    std::cout << "GLSL version: " << shadingLangVersion << std::endl;


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
    ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330"); // Adjust version as needed

    NodeEditor::Initialize();

    UpdateValuesFunc updateValuesFunc = std::bind(&Application::UpdateShadersValues, this, std::placeholders::_1);
    Shader::SetUpdateValuesFunc(updateValuesFunc);
    
    m_nodeWindow.Initialize();
    
    LoadEditorFile(EDITOR_FILE_NAME);

    std::cout << "Application initialized" << std::endl;
}

void Application::DrawMainDock()
{
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
    {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;
    ImGui::GetWindowDockID();

    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); // Set window background to red
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleColor();
    if (!opt_padding)
        ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    const ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        const ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }
    ImGui::End();
}

void Application::DrawMainBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New", "CTRL+N"))
            {
                m_nodeWindow.New();
            }
            if (ImGui::MenuItem("Open", "CTRL+O"))
            {
                std::filesystem::path savePath = std::filesystem::current_path() / SAVE_FOLDER;
                if (const std::string path = OpenDialog({ { "Node Editor", "node" } }, savePath.string().c_str()); !path.empty())
                {
                    m_nodeWindow.Load(path);
                }
            }
            if (ImGui::MenuItem("Save", "CTRL+S"))
            {
                m_nodeWindow.Save();
            }
            if (ImGui::MenuItem("Save As", "CTRL+SHIFT+S"))
            {
                std::filesystem::path savePath = std::filesystem::current_path() / SAVE_FOLDER;
                if (const std::string path = SaveDialog({ { "Node Editor", "node" } }, savePath.string().c_str()); !path.empty())
                {
                    m_nodeWindow.Save(path);
                }
            }
            if (ImGui::MenuItem("Export to shaderToy"))
            {
                ShaderMaker shaderMaker;
                shaderMaker.CreateShaderToyShader(m_nodeWindow.GetNodeManager());
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "ALT+F4"))
            {
                Application::Exit();
            }
            ImGui::EndMenu();
        }
        std::string fpsString = std::to_string(static_cast<int>(ImGui::GetIO().Framerate)) + " FPS";
        if (ImGui::BeginMenu(fpsString.c_str()))
        {
            
            ImGui::EndMenu();
        }

        /*
        std::string stateString = "Current State :" + UserInputEnumToString(m_nodeManager->GetUserInputState());
        if (ImGui::BeginMenu(stateString.c_str()))
        {
            ImGui::EndMenu();
        }
        */

        if (ImGui::BeginMenu("Debug"))
        {
            auto current = ActionManager::GetCurrent();
            if (current != nullptr)
            {
                auto redoneActions = ActionManager::GetRedoneActions();
                for (auto& redoneAction : redoneActions)
                {
                    if (ImGui::MenuItem(redoneAction->ToString().c_str()))
                    {
                        
                    }
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();
                        ImGui::Text("%p", redoneAction.get());
                        ImGui::EndTooltip();
                    }
                }
                ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 2);
                auto undoneActions = ActionManager::GetUndoneActions();
                for (uint32_t i = 0; i < undoneActions.size(); i++)
                {
                    if (ImGui::MenuItem(undoneActions[i]->ToString().c_str()))
                    {
                        
                    }
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();
                        ImGui::Text("%p", undoneActions[i].get());
                        ImGui::EndTooltip();
                    }
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void Application::WriteEditorFile(const std::string& path) const
{
    CppSer::Serializer serializer(path);
    serializer.SetVersion("1.0");
    serializer << CppSer::Pair::BeginMap << "Editor";
    serializer << CppSer::Pair::Key << "Node File" << CppSer::Pair::Value << m_nodeWindow.GetNodeManager()->GetFilePath().string();
    serializer << CppSer::Pair::EndMap << "Editor";
}

void Application::LoadEditorFile(const std::string& path) const
{
    auto fullPath = std::filesystem::path(path);
    CppSer::Parser parser(fullPath);
    if (!parser.IsFileOpen() || parser.GetVersion() != "1.0")
    {
        std::cout << "Invalid file" << std::endl;
        return;
    }
    m_nodeWindow.GetNodeManager()->LoadFromFile(parser["Node File"].As<std::string>());
}


void Application::Run()
{
    // Main loop
    while (!glfwWindowShouldClose(m_window))
    {
        m_time = glfwGetTime();
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DrawMainDock();

        DrawMainBar();

        m_nodeWindow.Draw();

        m_nodeWindow.Update();

        ImGui::Render();

        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_nodeWindow.Render();
        
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
    auto path = TEMP_FOLDER;
    std::filesystem::remove_all(path);
    
    WriteEditorFile(EDITOR_FILE_NAME);
    m_nodeWindow.Delete();
    
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
