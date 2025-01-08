#include "NodeWindow.h"

#include <complex>
#include <algorithm>
#include <filesystem>
#include <map>
#include <set>

#include <galaxymath/Maths.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <nfd.hpp>

#include "NodeSystem/NodeTemplateHandler.h"
#include "NodeSystem/LinkManager.h"
#include "NodeSystem/Node.h"
#include "NodeSystem/CustomNode.h"
#include "NodeSystem/ParamNode.h"
#include "NodeSystem/ShaderMaker.h"

#include "Application.h"
#include "Serializer.h"

#include "Render/Framebuffer.h"

#include "Actions/ActionCreateNode.h"
#include "Actions/ActionPaste.h"
#include "NodeSystem/RerouteNodeNamed.h"
class ParamNode;
using namespace GALAXY;

std::string SaveDialog(const std::vector<Filter>& filters, const char* defaultPath)
{
    std::string resultString;

    NFD::Guard nfdGuard;

    NFD::UniquePath outPath;

    const size_t count = filters.size();
    std::vector<nfdfilteritem_t> filterItems(count);

    for (size_t i = 0; i < count; i++)
    {
        filterItems[i].name = filters[i].name.c_str();
        filterItems[i].spec = filters[i].spec.c_str();
    }

    // show the dialog
    const nfdresult_t result = NFD::SaveDialog(outPath, filterItems.data(), static_cast<uint32_t>(count), defaultPath);
    if (result == NFD_OKAY)
    {
        resultString = std::string(outPath.get());
    }
    else if (result == NFD_CANCEL)
    {
    }
    else
    {
    }

    // NFD::Guard will automatically quit NFD.
    return resultString;
}

std::string OpenDialog(const std::vector<Filter>& filters, const char* defaultPath)
{
    std::string resultString;

    // initialize NFD
    NFD::Guard nfdGuard;

    // auto-freeing memory
    NFD::UniquePath outPath;

    const size_t count = filters.size();
    // prepare filters for the dialog
    std::vector<nfdfilteritem_t> filterItems(count);

    for (size_t i = 0; i < count; i++)
    {
        filterItems[i].name = filters[i].name.c_str();
        filterItems[i].spec = filters[i].spec.c_str();
    }

    // show the dialog

    const nfdresult_t result = NFD::OpenDialog(outPath, filterItems.data(), static_cast<uint32_t>(count), defaultPath);
    if (result == NFD_OKAY)
    {
        resultString = std::string(outPath.get());
    }
    else if (result == NFD_CANCEL)
    {
    }
    else
    {
    }

    // NFD::Guard will automatically quit NFD.
    return resultString;
}

void NodeWindow::Initialize()
{
    m_actionManager.SetContext(this);
    auto templateHandler = NodeTemplateHandler::Create();

    templateHandler->Initialize();
    
    m_nodeManager = new NodeManager(this);
    
    LoadEditorFile(EDITOR_FILE_NAME);

    m_quad = Application::GetInstance()->GetQuad();
    m_currentShader = std::make_shared<Shader>();
    m_currentShader->LoadDefaultShader();

    m_framebuffer = std::make_shared<Framebuffer>();
    m_framebuffer->Initialize();
}

void NodeWindow::PasteNode() const
{
    auto action = std::make_shared<ActionPaste>(m_nodeManager, m_gridWindow.zoom, m_gridWindow.origin, ImGui::GetMousePos(), ImGui::GetClipboardText());
    ActionManager::DoAction(action);
}

bool Splitter(const bool split_vertically, const float thickness, float* size1, float* size2, const float min_size1, const float min_size2, const float splitter_long_axis_size /*= -1.0f*/)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    const ImGuiID id = window->GetID("##Splitter");
    ImRect bb;
    bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
    bb.Max = bb.Min + ImGui::CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
    return ImGui::SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}

void NodeWindow::Update() const
{
    if (NodeTemplateHandler* nodeTemplateHandler = NodeTemplateHandler::GetInstance())
    {
        nodeTemplateHandler->ComputeNodesSize();
    }

    if (!m_isFocused)
        return;
    
    m_nodeManager->UpdateNodes(m_gridWindow.zoom, m_gridWindow.origin, ImGui::GetMousePos());
    
    m_nodeManager->RightClickStreamMenuUpdate();

    m_nodeManager->CreateNodeMenuUpdate(m_gridWindow.zoom, m_gridWindow.origin, ImGui::GetMousePos());

    if (ImGui::IsKeyPressed(ImGuiKey_C) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
    {
        CppSer::Serializer serializer;
        m_nodeManager->SerializeSelectedNodes(serializer);
        ImGui::SetClipboardText(serializer.GetContent().c_str());
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_V) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
    {
        PasteNode();
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Z) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
    {
        ActionManager::UndoLastAction();
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_Y) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
    {
        ActionManager::RedoLastAction();
    }
}

void NodeWindow::Delete() const
{
    WriteEditorFile(EDITOR_FILE_NAME);
    m_nodeManager->Clean();
    delete m_nodeManager;
}

void NodeWindow::DrawMainDock()
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

void NodeWindow::Draw()
{
    DrawMainDock();
    DrawMainBar();

    if (ImGui::Begin("Node Editor", nullptr))
    {
        m_isFocused = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) || ImGui::IsWindowFocused(ImGuiHoveredFlags_RootAndChildWindows);
        // ImGui::SetWindowPos({ 0, 0 });
        // Vec2f windowSize = Application::GetInstance()->GetWindowSize();
        // ImGui::SetWindowSize({ windowSize.x, windowSize.y });
        // ImGui::SetWindowCollapsed(false);

        DrawInspector();
        ImGui::SameLine();
        DrawGrid();
    }
}

void NodeWindow::DrawNodePreview(const std::shared_ptr<Node> previewNode)
{
    previewNode->m_framebuffer->Update();
    previewNode->m_framebuffer->Bind();
    previewNode->m_shader->Use();
    previewNode->m_shader->UpdateValues();
    m_quad->Draw();
    previewNode->m_framebuffer->Unbind();
}

void NodeWindow::Render()
{
    // UpdateShader();
    UpdateShaders();

    for (auto it = m_previewNodes.begin(); it != m_previewNodes.end();)
    {
        const auto previewNode = m_nodeManager->GetNode(*it).lock();

        if (!previewNode || !previewNode->p_preview)
        {
            it = m_previewNodes.erase(it); // Erase returns the next valid iterator
            continue;
        }
        previewNode->RenderPreview(m_quad);
        ++it;
    }

    m_framebuffer->Update();
    m_framebuffer->Bind();
    m_currentShader->Use();
    m_currentShader->UpdateValues();
    m_quad->Draw();
    m_framebuffer->Unbind();
}

void NodeWindow::ResetActionManager()
{
    m_actionManager = ActionManager();
    m_actionManager.SetContext(this);
}

void NodeWindow::DrawMainBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New", "CTRL+N"))
            {
                m_nodeManager->Clean();
                delete m_nodeManager;
                m_nodeManager = new NodeManager(this);
                ResetActionManager();
            }
            if (ImGui::MenuItem("Open", "CTRL+O"))
            {
                std::filesystem::path savePath = std::filesystem::current_path() / SAVE_FOLDER;
                if (const std::string path = OpenDialog({ { "Node Editor", "node" } }, savePath.string().c_str()); !path.empty())
                {
                    m_nodeManager->LoadFromFile(path);
                    ResetActionManager();
                }
            }
            if (ImGui::MenuItem("Save", "CTRL+S"))
            {
                std::filesystem::path savePath = std::filesystem::current_path() / SAVE_FOLDER;
                std::string path = m_nodeManager->GetFilePath().generic_string();

                if (path.empty())
                    path = savePath.string();
                if (!path.empty())
                {
                    m_nodeManager->SaveToFile(path);
                }
            }
            if (ImGui::MenuItem("Save As", "CTRL+SHIFT+S"))
            {
                std::filesystem::path savePath = std::filesystem::current_path() / SAVE_FOLDER;
                if (const std::string path = SaveDialog({ { "Node Editor", "node" } }, savePath.string().c_str()); !path.empty())
                {
                    m_nodeManager->SaveToFile(path);
                }
            }
            if (ImGui::MenuItem("Export to shaderToy"))
            {
                ShaderMaker shaderMaker;
                shaderMaker.CreateShaderToyShader(m_nodeManager);
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
        
        std::string stateString = "Current State :" + UserInputEnumToString(m_nodeManager->GetUserInputState());
        if (ImGui::BeginMenu(stateString.c_str()))
        {
            ImGui::EndMenu();
        }

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

void NodeWindow::WriteEditorFile(const std::string& path) const
{
    CppSer::Serializer serializer(path);
    serializer.SetVersion("1.0");
    serializer << CppSer::Pair::BeginMap << "Editor";
    serializer << CppSer::Pair::Key << "Node File" << CppSer::Pair::Value << m_nodeManager->GetFilePath().string();
    serializer << CppSer::Pair::EndMap << "Editor";
}

void NodeWindow::LoadEditorFile(const std::string& path) const
{
    auto fullPath = std::filesystem::path(path);
    CppSer::Parser parser(fullPath);
    if (!parser.IsFileOpen() || parser.GetVersion() != "1.0")
    {
        std::cout << "Invalid file" << std::endl;
        return;
    }
    m_nodeManager->LoadFromFile(parser["Node File"].As<std::string>());
}

void NodeWindow::DrawInspector() const
{
    static float size1 = 300.f;
    static float size2 = ImGui::GetContentRegionAvail().y;
    Splitter(true, 5.f, &size1, &size2, 100.f, 0.f, ImGui::GetContentRegionAvail().y);

    ImGui::BeginChild("##inspector", ImVec2(size1, 0), true);
    
    ImGui::Text("Inspector");

    ImGui::Separator();
    
    Vec2f size = {size1 - 15, size1 - 15};
    m_framebuffer->SetNewSize(size);
    ImGui::Dummy(Vec2f(5, 5));
    ImGui::Image(reinterpret_cast<void*>(static_cast<size_t>(m_framebuffer->GetRenderTexture())), size, ImVec2(0, 1), ImVec2(1, 0), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
    ImGui::Separator();

    if (NodeRef selectedNode = m_nodeManager->GetSelectedNode().lock())
    {
        selectedNode->ShowInInspector();
    }
    
    ImGui::EndChild();   
}

void NodeWindow::UpdateShader()
{
    if (m_shouldUpdateShader)
    {
        ShaderMaker shaderMaker;
        std::string content;
        
        shaderMaker.CreateFragmentShader(content, m_nodeManager);
        
        m_currentShader->RecompileFragmentShader(content.c_str());
        
        m_shouldUpdateShader = false;
    }
}

void NodeWindow::UpdateShaders()
{
    if (m_shouldUpdateShader)
    {
        ShaderMaker shaderMaker;
        
        shaderMaker.DoWork(m_nodeManager);
        
        std::string content;
        
        shaderMaker.CreateFragmentShader(content, m_nodeManager);
        
        m_currentShader->RecompileFragmentShader(content.c_str());
        
        m_shouldUpdateShader = false;
    }
}

void NodeWindow::DrawGrid()
{
    if (ImGui::IsWindowFocused())
    {
        ActionManager::SetCurrent(&m_actionManager);
    }

    static ImVec2 scrolling(ImGui::GetContentRegionAvail().x / 2.0f, ImGui::GetContentRegionAvail().y / 2.0f);
    // static ImVec2 scrolling(0.0f, 0.0f);
    
    float& zoom = m_gridWindow.zoom;
    Vec2f& origin = m_gridWindow.origin;
    
    // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
    ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
    ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
    canvas_sz.x = std::max(canvas_sz.x, 50.0f);
    canvas_sz.y = std::max(canvas_sz.y, 50.0f);
    ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

    // Draw border and background color
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
    // draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

    // This will catch our interactions
    ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    const bool is_hovered = ImGui::IsItemHovered(); // Hovered
    const bool is_active = ImGui::IsItemActive();   // Held// Adjust the origin based on zoom
    origin = {canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y};
    // Adjust mouse position with zoom
    const ImVec2 mousePos(io.MousePos.x, io.MousePos.y);

    // draw_list->AddCircleFilled(mousePos, 5, IM_COL32(255, 0, 0, 255));
    // draw_list->AddCircleFilled(origin, 5, IM_COL32(0, 0, 0, 255));
    
    // Zoom with mouse wheel
    if (is_hovered)
    {
        if (io.MouseWheel != 0.0f )
        {
            // Get mouse position relative to the origin (before zoom)
            ImVec2 mouse_offset = ImVec2((mousePos.x - origin.x) / zoom, (mousePos.y - origin.y) / zoom);

            // Calculate zoom scale based on current zoom
            float zoom_scale = 0.1f * zoom;

            // Update zoom and clamp to a minimum value
            zoom += io.MouseWheel * zoom_scale;
            zoom = zoom < 0.1f ? 0.1f : zoom;

            // Recalculate origin to zoom towards the mouse position
            scrolling.x = mousePos.x - mouse_offset.x * zoom - canvas_p0.x;
            scrolling.y = mousePos.y - mouse_offset.y * zoom - canvas_p0.y;

            // Update origin 
            origin = {canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y};
        }
    }
    // Pan (we use a zero mouse threshold when there's no context menu)
    // You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
    constexpr float mouse_threshold_for_pan = -1.0f;
    if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
    {
        scrolling.x += io.MouseDelta.x;
        scrolling.y += io.MouseDelta.y;
    }

    // Draw grid + all lines in the canvas
    draw_list->PushClipRect(canvas_p0, canvas_p1, true);
    m_nodeManager->m_isGridHovered = is_hovered;
    
    const float GRID_STEP = 64.0f * zoom; // Adjust the grid step based on zoom
    for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
        draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
    for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
        draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));


    m_nodeManager->DrawNodes(zoom, origin, mousePos);
    m_nodeManager->EOnDrawEvent.Invoke();
    
    draw_list->PopClipRect();


}
