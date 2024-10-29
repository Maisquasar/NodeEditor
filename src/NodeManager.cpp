#include "NodeManager.h"
#include <ranges>

#include "Node.h"

void NodeManager::AddNode(NodeRef node)
{
    node->AddInput("Input", Type::Float);
    node->AddInput("Input", Type::Float);
    node->AddInput("Input", Type::Float);
    node->AddOutput("Output", Type::Float);
    node->AddOutput("Output", Type::Float);
    node->AddOutput("Output", Type::Float);
    m_nodes[node->p_uuid] = node;
    node->p_nodeManager = this;
}

void NodeManager::DrawNodes(float zoom, const Vec2f& origin, const Vec2f& mousePos)
{
    m_zoom = zoom;
    m_origin = origin;
    
    bool mouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    NodeRef selectedNode = m_selectedNode.lock();
    bool wasNodeClicked = false;
    bool wasInputClicked = false;
    for (const NodeRef& node : m_nodes | std::views::values)
    {
        bool alreadyOneSelected = selectedNode != m_selectedNode.lock();
        node->Draw(zoom, origin);

        if (mouseClicked)
        {
            for (int i = 0; i < node->p_outputs.size(); i++)
            {
                Vec2f circlePos = node->GetOutputPosition(i);
                if (node->IsPointInsideCircle(mousePos, circlePos, origin, zoom, i))
                {
                    m_selectedOutput = node->GetOutput(i);
                    alreadyOneSelected = true;
                    wasInputClicked = true;
                    break;
                }
            }
        }

        if (mouseClicked && !alreadyOneSelected && node->IsPointInNode(mousePos, origin, zoom))
        {
            SelectNode(node);
            
            m_firstClickOffset = mousePos;
            m_defaultPosition = node->p_position;

            m_selectedOutput.reset();

            wasNodeClicked = true;
        }
    }
    
    if (mouseClicked && !wasNodeClicked && !wasInputClicked)
    {
        SelectNode(nullptr);
    }

    if (auto selectedOutput = m_selectedOutput.lock())
    {
        NodeWeakRef nodeWeakRef = GetNode(selectedOutput->parentUUID);
        Vec2f inPosition = nodeWeakRef.lock()->GetOutputPosition(selectedOutput->index, origin, zoom);
        /*ImGui::GetWindowDrawList()->AddBezierCubic(
            inPosition, inPosition ,
            mousePos, mousePos ,
            ImColor(255, 255, 255), 3, 12);*/
        ImGui::GetWindowDrawList()->AddLine(inPosition, mousePos, ImColor(255, 255, 255), 3);
    }

    NodeRef currentSelectedNode = m_selectedNode.lock();
    // Move selected node
    if (!m_selectedOutput.lock() && currentSelectedNode && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        Vec2f newPosition = m_defaultPosition + (mousePos - m_firstClickOffset) / zoom;
        currentSelectedNode->SetPosition(newPosition);
    }
}

void NodeManager::SelectNode(NodeRef node)
{
    if (auto selectedNode = m_selectedNode.lock())
        selectedNode->p_selected = false;
    
    m_selectedNode = node;

    if (auto selectedNode = m_selectedNode.lock())
        selectedNode->p_selected = true;
    
}
