#include "MainWindow.h"


#include <galaxymath/Maths.h>
using namespace GALAXY;
#include <imgui.h>

#include "Node.h"


void MainWindow::Initialize()
{
    m_nodeManager = std::make_unique<NodeManager>();
}

void MainWindow::Draw()
{
    DrawGrid();
}

void MainWindow::Delete()
{
}

void MainWindow::DrawGrid()
{
    static ImVec2 scrolling(0.0f, 0.0f);
    static bool opt_enable_grid = true;
    static bool opt_enable_context_menu = true;
    static float zoom = 1.0f; // Add a zoom variable
    
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
    const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y);
    // Adjust mouse position with zoom
    const ImVec2 mousePos(io.MousePos.x, io.MousePos.y);

    ImGui::GetWindowDrawList()->AddCircleFilled(mousePos, 5, IM_COL32(255, 0, 0, 255));
    ImGui::GetWindowDrawList()->AddCircleFilled(origin, 5, IM_COL32(0, 0, 255, 255));

    
    // Zoom with mouse wheel
    if (is_hovered)
    {

        if (io.MouseWheel != 0.0f )
        {
            zoom += io.MouseWheel * 0.1f;
            zoom = zoom < 0.1f ? 0.1f : zoom; // Prevent zoom from going too small
            // scrolling.x += mouse_pos_in_canvas.x * zoom * 0.5f;
            // scrolling.y += mouse_pos_in_canvas.y * zoom * 0.5f;
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
        if (ImGui::Button("Add Node"))
        {
            NodeRef node = std::make_shared<Node>("Node");
            node->SetPosition((mousePos - origin) / zoom);
            m_nodeManager->AddNode(node);
            
            ImGui::CloseCurrentPopup();
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

    std::cout << "Zoom: " << zoom << std::endl;

    m_nodeManager->DrawNodes(zoom, origin, mousePos);
    
    draw_list->PopClipRect();
}
