#include "MainWindow.h"

#include <filesystem>
#include <map>
#include <galaxymath/Maths.h>

#include "NodeTemplateHandler.h"
#include "Serializer.h"
#include "Actions/ActionCreateNode.h"
#include "Actions/ActionPaste.h"
using namespace GALAXY;

#include "Application.h"
#include "LinkManager.h"
#include <imgui.h>

#include "Node.h"

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

void MainWindow::Initialize()
{
    auto templateHandler = NodeTemplateHandler::Create();

    templateHandler->Initialize();
    
    m_nodeManager = new NodeManager(this);
}

void MainWindow::PasteNode() const
{
    ActionPaste* action = new ActionPaste(m_nodeManager, m_gridWindow.zoom, m_gridWindow.origin, ImGui::GetMousePos(), ImGui::GetClipboardText());
    ActionManager::DoAction(action);
}



void MainWindow::Update() const
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

void MainWindow::Draw()
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
                m_actionManager = ActionManager();
            }
            if (ImGui::MenuItem("Open", "CTRL+O"))
            {
                std::filesystem::path savePath = std::filesystem::current_path() / SAVE_FOLDER;
                if (const std::string path = OpenDialog({ { "Node Editor", "node" } }, savePath.string().c_str()); !path.empty())
                {
                    m_nodeManager->LoadFromFile(path);
                    m_actionManager = ActionManager();
                }
            }
            if (ImGui::MenuItem("Save", "CTRL+S"))
            {
                std::filesystem::path savePath = std::filesystem::current_path() / SAVE_FOLDER;
                if (const std::string path = SaveDialog({ { "Node Editor", "node" } }, savePath.string().c_str()); !path.empty())
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
    DrawGrid();
}

void MainWindow::Delete() const
{
    m_nodeManager->Clean();
    delete m_nodeManager;
}

void MainWindow::DrawContextMenu(float& zoom, Vec2f& origin, const ImVec2 mousePos) const
{
    constexpr uint32_t displayCount = 10;
    // Context menu
    if (ImGui::BeginPopup("context"))
    {
        static ImGuiTextFilter filter;

        filter.Draw("", ImGui::GetContentRegionAvail().x);
        auto nodeTemplate = NodeTemplateHandler::GetInstance();
        auto templates = nodeTemplate->GetTemplates();
        uint32_t j = 0;
        for (uint32_t i = 0; i < templates.size() && j < displayCount; i++)
        {
            std::string name = templates[i]->GetName();
            if (!filter.PassFilter(name.c_str()) || !templates[i]->GetAllowInteraction())
                continue;
            if (ImGui::MenuItem(name.c_str()))
            {
                const uint32_t templateId = templates[i]->GetTemplateID();
                
                NodeRef node = NodeTemplateHandler::CreateFromTemplate(templateId);
                ActionCreateNode* action = new ActionCreateNode(m_nodeManager, node);
                
                ActionManager::AddAction(action);
                
                node->SetPosition((mousePos - origin) / zoom);
                m_nodeManager->AddNode(node);
                break;
            }
            j++;
        }
        ImGui::EndPopup();
    }
}

void MainWindow::DrawGrid()
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
    draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

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
        ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);

    DrawContextMenu(zoom, origin, mousePos);

    // Draw grid + all lines in the canvas
    draw_list->PushClipRect(canvas_p0, canvas_p1, true);
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
