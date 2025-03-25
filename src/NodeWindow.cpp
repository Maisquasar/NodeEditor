#include "NodeWindow.h"

#include <complex>
#include <algorithm>
#include <CppSerializer.h>
#include <filesystem>
#include <map>
#include <set>

#include <galaxymath/Maths.h>
using namespace GALAXY;
#include <imgui.h>
#include <imgui_internal.h>

#include "NodeSystem/NodeTemplateHandler.h"
#include "NodeSystem/LinkManager.h"
#include "NodeSystem/Node.h"
#include "NodeSystem/CustomNode.h"
#include "NodeSystem/ParamNode.h"
#include "NodeSystem/ShaderMaker.h"

#include "Serializer.h"

#include "Render/Framebuffer.h"

#include "Actions/ActionCreateNode.h"
#include "Actions/ActionPaste.h"
#include "NodeSystem/RerouteNodeNamed.h"
class ParamNode;
using namespace GALAXY;

void NodeWindow::Initialize()
{
    m_actionManager.SetContext(this);
    
    m_nodeManager = new NodeManager(this);

    m_quad = Mesh::GetQuad();
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
    m_nodeManager->Clean();
    delete m_nodeManager;
}

void NodeWindow::Draw()
{
    if (ImGui::Begin(m_windowName.c_str(), nullptr))
    {
        m_ImGuiWindow = ImGui::GetCurrentWindow();
        m_isFocused = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) || ImGui::IsWindowFocused(ImGuiHoveredFlags_RootAndChildWindows);

        DrawInspector();
        ImGui::SameLine();
        DrawGrid();
    }
}

void NodeWindow::RenderNodePreview(const std::shared_ptr<Node> previewNode)
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
    UpdateShaders();
}

void NodeWindow::ResetActionManager()
{
    m_actionManager = ActionManager();
    m_actionManager.SetContext(this);
}

bool NodeWindow::LoadScene(const std::filesystem::path& path)
{
    if (m_nodeManager->LoadFromFile(path))
    {
        ResetActionManager();
        return true;
    }
    return false;
}

bool NodeWindow::SaveScene(const std::filesystem::path& path)
{
    return m_nodeManager->SaveToFile(path);
}

bool NodeWindow::SaveScene()
{
    return m_nodeManager->SaveToFile(m_nodeManager->GetFilePath());
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


        // Material Node Shader
        std::string content;
        
        shaderMaker.CreateFragmentShader(content, m_nodeManager);
        
        // ImGui::SetClipboardText(content.c_str());
        
        m_currentShader->RecompileFragmentShader(content.c_str());

        for (auto& node : m_nodeManager->m_nodes)
        {
            if (auto paramNode = std::dynamic_pointer_cast<ParamNode>(node.second))
            {
                m_currentShader->SendValue(paramNode->GetParamName().c_str(), paramNode->GetPreviewValue(), paramNode->GetType());
            }
        }

        m_framebuffer->Update();
        m_framebuffer->Bind();
        m_currentShader->Use();
        m_currentShader->UpdateValues();
        m_quad->Draw();
        m_framebuffer->Unbind();
        
        m_shouldUpdateShader = false;
    }
}

void NodeWindow::NewScene()
{
    m_nodeManager->Clean();
    delete m_nodeManager;
    m_nodeManager = new NodeManager(this);
    ResetActionManager();
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
    
    draw_list->PopClipRect();


}
