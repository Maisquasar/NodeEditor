#include "MainWindow.h"

#include <filesystem>
#include <map>
#include <galaxymath/Maths.h>

#include "NodeTemplateHandler.h"
#include "Serializer.h"
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
    
    m_nodeManager = new NodeManager();
}

void MainWindow::PasteNode() const
{
    std::string clipboardText = ImGui::GetClipboardText();
    CppSer::Parser parser = CppSer::Parser(clipboardText);
        
    SerializedData data = NodeManager::DeserializeData(parser);

    // Calculate the bounding box of copied nodes
    Vec2f minPos(FLT_MAX, FLT_MAX);
    Vec2f maxPos(-FLT_MAX, -FLT_MAX);
    for (const auto& node : data.nodes)
    {
        // Adjust for zoom and origin when calculating node bounds
        Vec2f min = node->GetMin(m_gridWindow.zoom, m_gridWindow.origin);
        Vec2f max = node->GetMax(min, m_gridWindow.zoom);
        minPos.x = std::min(minPos.x, min.x);
        minPos.y = std::min(minPos.y, min.y);
        maxPos.x = std::max(maxPos.x, max.x);
        maxPos.y = std::max(maxPos.y, max.y);
    }
        
    // Calculate the local center of the copied nodes
    Vec2f localCenter = (minPos + maxPos) * 0.5f;
        
    // Get the mouse position as the paste position, adjusted for zoom and origin
    Vec2f mousePos = ImGui::GetMousePos();
    Vec2f pastePosition = NodeManager::ToGrid(mousePos, m_gridWindow.zoom, m_gridWindow.origin);
        
    // Map old UUIDs to new UUIDs after resetting them
    std::map<UUID, UUID> uuidMap;
    for (auto& node : data.nodes)
    {
        UUID oldUUID = node->GetUUID();
        node->ResetUUID();
        uuidMap[oldUUID] = node->GetUUID();
            
        // Adjust node position relative to the local center and paste position, taking zoom and origin into account
        Vec2f offset = (node->GetPosition() - NodeManager::ToGrid(localCenter, m_gridWindow.zoom, m_gridWindow.origin));
        node->SetPosition(pastePosition + offset);
            
        // Add node to node manager
        m_nodeManager->AddNode(node);
    }

    // Adjust link references to new node UUIDs
    for (auto& link : data.links)
    {
        if (uuidMap.contains(link->fromNodeIndex))
            link->fromNodeIndex = uuidMap[link->fromNodeIndex];
        if (uuidMap.contains(link->toNodeIndex))
            link->toNodeIndex = uuidMap[link->toNodeIndex];
            
        // Add link to link manager
        m_nodeManager->GetLinkManager()->AddLink(link);
    }
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
                m_nodeManager = new NodeManager();
            }
            if (ImGui::MenuItem("Open", "CTRL+O"))
            {
                std::filesystem::path savePath = std::filesystem::current_path() / SAVE_FOLDER;
                if (const std::string path = OpenDialog({ { "Node Editor", "node" } }, savePath.string().c_str()); !path.empty())
                {
                    m_nodeManager->LoadFromFile(path);
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

void MainWindow::DrawGrid()
{
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

    // Context menu
    if (ImGui::BeginPopup("context"))
    {
        auto nodeTemplate = NodeTemplateHandler::GetInstance();
        auto templates = nodeTemplate->GetTemplates();
        for (int i = 0; i < templates.size(); i++)
        {
            if (!templates[i]->GetAllowInteraction())
                continue;
            if (ImGui::MenuItem(templates[i]->GetName().c_str()))
            {
                uint32_t templateId = templates[i]->GetTemplateID();
                NodeRef node = nodeTemplate->CreateFromTemplate(templateId);
                node->SetPosition((mousePos - origin) / zoom);
                m_nodeManager->AddNode(node);
            }
        }
        ImGui::EndPopup();
    }

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
