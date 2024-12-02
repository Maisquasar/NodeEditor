#include "NodeWindow.h"

#include <complex.h>
#include <filesystem>
#include <map>
#include <set>
#include <galaxymath/Maths.h>

#include "NodeSystem/NodeTemplateHandler.h"
#include "NodeSystem/LinkManager.h"
#include "NodeSystem/Node.h"

#include "Application.h"
#include "Serializer.h"
#include "NodeSystem/ShaderMaker.h"

#include "Actions/ActionCreateNode.h"
#include "Actions/ActionPaste.h"
#include "NodeSystem/CustomNode.h"
#include "NodeSystem/ParamNode.h"
#include "Render/Framebuffer.h"
class ParamNode;
using namespace GALAXY;

#include <imgui.h>
#include <imgui_internal.h>

#include <nfd.hpp>


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
    m_currentShader->Load("shaders/shader");

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
    m_nodeManager->UpdateNodes(m_gridWindow.zoom, m_gridWindow.origin, ImGui::GetMousePos());

    if (ImGui::IsKeyPressed(ImGuiKey_C) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
    {
        auto serializer = CppSer::Serializer();
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
        // ImGui::SetWindowPos({ 0, 0 });
        // Vec2f windowSize = Application::GetInstance()->GetWindowSize();
        // ImGui::SetWindowSize({ windowSize.x, windowSize.y });
        // ImGui::SetWindowCollapsed(false);

        DrawInspector();
        ImGui::SameLine();
        DrawGrid();
    }
}

void NodeWindow::Render()
{
    UpdateShader();

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
                auto actions = current->GetUndoneActions();
                for (uint32_t i = 0; i < actions.size(); i++)
                {
                    if (ImGui::MenuItem(actions[i]->ToString().c_str()))
                    {
                        
                    }
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();
                        ImGui::Text("%p", (void*)actions[i].get());
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
    ImGui::Image(reinterpret_cast<ImTextureID>(m_framebuffer->GetRenderTexture()), size, ImVec2(0, 1), ImVec2(1, 0), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
    ImGui::Separator();

    if (NodeRef selectedNode = m_nodeManager->GetSelectedNode().lock())
    {
        selectedNode->ShowInInspector();
    }
    
    ImGui::EndChild();   
}

void NodeWindow::DrawContextMenu(float& zoom, Vec2f& origin, const ImVec2 mousePos)
{
    constexpr uint32_t displayCount = 10;

    // Context menu
    if (m_contextOpen = ImGui::BeginPopup("context"); m_contextOpen)
    {
        static ImGuiTextFilter filter("");

        if (m_shouldOpenContextMenu == 0)
        {
            m_shouldOpenContextMenu = -1;
            ImGui::CloseCurrentPopup();
        }

        if (m_focusInput)
        {
            ImGui::SetKeyboardFocusHere();
            m_focusInput = false;
        }
        filter.Draw("", ImGui::GetContentRegionAvail().x);
        auto nodeTemplate = NodeTemplateHandler::GetInstance();
        auto templates = nodeTemplate->GetTemplates();

        // If is linking
        bool isLinking = m_nodeManager->CurrentLinkIsAlmostLinked();

        StreamRef streamLinking = nullptr;
        bool isOutput = false;
        if (isLinking)
        {
            Link currentLink = m_nodeManager->GetCurrentLink();
            if (currentLink.fromNodeIndex != UUID_NULL && currentLink.fromOutputIndex != UUID_NULL)
            {
                streamLinking = m_nodeManager->GetOutput(currentLink.fromNodeIndex, currentLink.fromOutputIndex).lock();
                isOutput = true;
            }
            else if (currentLink.toNodeIndex != UUID_NULL && currentLink.toInputIndex != UUID_NULL)
            {
                streamLinking = m_nodeManager->GetInput(currentLink.toNodeIndex, currentLink.toInputIndex).lock();
                isOutput = false;
            }
            else
            {
                isLinking = false;
            }
        }

        ImGui::BeginChild("##context", ImVec2(0, 350));
        uint32_t j = 0;
        for (uint32_t i = 0; i < templates.size()/* && j < displayCount*/; i++)
        {
            std::string name = templates[i].node->GetName();
            if (!templates[i].node->GetAllowInteraction())
                continue;
            bool passFilter = false;
            if (filter.PassFilter(name.c_str()))
                passFilter = true;
            for (uint32_t j = 0; j < templates[i].searchStrings.size(); j++)
            {
                if (passFilter)
                    break;
                if (filter.PassFilter(templates[i].searchStrings[j].c_str()))
                {
                    passFilter = true;
                    break;
                }
            }
            if (!passFilter)
                continue;
            if (isLinking)
            {
                if (isOutput && !templates[i].node->p_inputs.empty() && templates[i].node->p_inputs[0]->type != streamLinking->type && !templates[i].node->p_alwaysVisibleOnContext)
                    continue;
                if (!isOutput && !templates[i].node->p_outputs.empty() && templates[i].node->p_outputs[0]->type != streamLinking->type&& !templates[i].node->p_alwaysVisibleOnContext)
                    continue;
                if (!isOutput && templates[i].node->p_outputs.empty() || isOutput && templates[i].node->p_inputs.empty())
                    continue;
            }
            if (ImGui::MenuItem(name.c_str()) || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))
            {
                const TemplateID templateId = templates[i].node->GetTemplateID();
                
                NodeRef node = NodeTemplateHandler::CreateFromTemplate(templateId);
                node->p_nodeManager = m_nodeManager;

                if (isLinking)
                {
                    if (auto customNode = std::dynamic_pointer_cast<CustomNode>(node))
                    {
                        if (isOutput)
                        {
                            customNode->ClearInputs();
                            customNode->AddInput("In", streamLinking->type);
                        }
                        else
                        {
                            customNode->ClearOutputs();
                            customNode->AddOutput("Out", streamLinking->type);
                        }
                    }
                    else if (auto paramNode = std::dynamic_pointer_cast<ParamNode>(node))
                    {
                        paramNode->SetType(streamLinking->type);
                    }
                }
                
                auto action = std::make_shared<ActionCreateNode>(m_nodeManager, node);
                
                ActionManager::AddAction(action);
                
                node->SetPosition((m_mousePosOnContext - origin) / zoom);
                m_nodeManager->AddNode(node);

                filter.Clear();
                if (isLinking)
                {
                    Link& link = m_nodeManager->GetCurrentLink();
                    if (std::dynamic_pointer_cast<Output>(streamLinking))
                    {
                        link.toInputIndex = 0;
                        link.toNodeIndex = node->GetUUID();
                    }
                    else if (std::dynamic_pointer_cast<Input>(streamLinking))
                    {
                        link.fromOutputIndex = 0;
                        link.fromNodeIndex = node->GetUUID();
                    }
                }
                ImGui::CloseCurrentPopup();
                break;
            }
            j++;
        }
        ImGui::EndChild();
        ImGui::EndPopup();
    }
}

void NodeWindow::SetOpenContextMenu(bool shouldOpen)
{
    m_shouldOpenContextMenu = shouldOpen;
}

void NodeWindow::UpdateShader()
{
    if (m_shouldUpdateShader)
    {
        ShaderMaker shaderMaker;
        shaderMaker.CreateFragmentShader("shaders/shader.frag", m_nodeManager);
        m_currentShader->RecompileFragmentShader();
        
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
    static bool opt_enable_grid = true;
    static bool opt_enable_context_menu = true;
    
    float& zoom = m_gridWindow.zoom;
    Vec2f& origin = m_gridWindow.origin;
    
    // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
    ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
    ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
    if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
    if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
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
    const float mouse_threshold_for_pan = opt_enable_context_menu ? -1.0f : 0.0f;
    if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
    {
        scrolling.x += io.MouseDelta.x;
        scrolling.y += io.MouseDelta.y;
    }

    // Context menu (under default mouse threshold)
    ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    if (opt_enable_context_menu && drag_delta.x == 0.0f && drag_delta.y == 0.0f)
    {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            // m_shouldOpenContextMenu = true;
            m_mousePosOnContext = mousePos;
        }
        ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
        bool open = ImGui::IsItemClicked(ImGuiPopupFlags_MouseButtonRight);
        if (m_shouldOpenContextMenu == 1 || open)
        {
            m_shouldOpenContextMenu = -1;
            m_mousePosOnContext = mousePos;
            m_focusInput = true;
            if (!open)
                ImGui::OpenPopup("context");
        }
    }

    DrawContextMenu(zoom, origin, mousePos);

    // Draw grid + all lines in the canvas
    draw_list->PushClipRect(canvas_p0, canvas_p1, true);
    m_nodeManager->m_isGridHovered = ImGui::IsMouseHoveringRect(canvas_p0, canvas_p1);
    if (opt_enable_grid)
    {
        const float GRID_STEP = 64.0f * zoom; // Adjust the grid step based on zoom
        for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
            draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
        for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
            draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
    }

    m_nodeManager->DrawNodes(zoom, origin, mousePos);
    m_nodeManager->EOnDrawEvent.Invoke();
    
    draw_list->PopClipRect();


}
