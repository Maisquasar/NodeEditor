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

void NodeManager::UpdateToLink(float zoom, const Vec2f& origin, const Vec2f& mousePos, bool mouseClicked,
                               const OutputRef& selectedOutput, const NodeRef& node) const
{
    if (mouseClicked && selectedOutput)
    {
        for (size_t i = 0; i < node->p_inputs.size(); i++)
        {
            Vec2f circlePos = node->GetInputPosition(i);
            if (node->IsPointHoverCircle(mousePos, circlePos, origin, zoom, i))
            {
                Link link;
                link.toNodeIndex = node->p_uuid;
                link.toOutputIndex = static_cast<uint32_t>(i);
                link.fromOutputIndex = selectedOutput->index;
                NodeRef outputNode = GetNode(selectedOutput->parentUUID).lock();
                outputNode->AddLink(link);
                std::cout << "Added link : " << link.fromOutputIndex << " -> " << link.toOutputIndex << std::endl;
                break;
            }
        }
    }
}

void NodeManager::UpdateFromLink(float zoom, const Vec2f& origin, const Vec2f& mousePos, bool mouseClicked,
                                 bool& wasInputClicked, NodeRef& node, bool& alreadyOneSelected)
{
    if (mouseClicked && !m_selectedOutput.lock())
    {
        for (size_t i = 0; i < node->p_outputs.size(); i++)
        {
            Vec2f circlePos = node->GetOutputPosition(i);
            if (node->IsPointHoverCircle(mousePos, circlePos, origin, zoom, i))
            {
                m_selectedOutput = node->GetOutput(i);
                alreadyOneSelected = true;
                wasInputClicked = true;
                break;
            }
        }
    }
}

void NodeManager::UpdateNodes(float zoom, const Vec2f& origin, const Vec2f& mousePos)
{
    m_zoom = zoom;
    m_origin = origin;
    
    bool mouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    NodeRef selectedNode = m_selectedNode.lock();
    OutputRef selectedOutput = m_selectedOutput.lock();
    bool wasNodeClicked = false;
    bool wasInputClicked = false;
    for (NodeRef& node : m_nodes | std::views::values)
    {
        bool alreadyOneSelected = selectedNode != m_selectedNode.lock();

        UpdateToLink(zoom, origin, mousePos, mouseClicked, selectedOutput, node);

        UpdateFromLink(zoom, origin, mousePos, mouseClicked, wasInputClicked, node, alreadyOneSelected);

        if (mouseClicked && !alreadyOneSelected && node->IsPointHoverNode(mousePos, origin, zoom))
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

    NodeRef currentSelectedNode = m_selectedNode.lock();
    // Move selected node
    if (!m_selectedOutput.lock() && currentSelectedNode && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        Vec2f newPosition = m_defaultPosition + (mousePos - m_firstClickOffset) / zoom;
        currentSelectedNode->SetPosition(newPosition);
    }
}

void NodeManager::DrawNodes(float zoom, const Vec2f& origin, const Vec2f& mousePos) const
{
    for (const NodeRef& node : m_nodes | std::views::values)
    {
        node->Draw(zoom, origin);
    }

    for (const NodeRef& node : m_nodes | std::views::values)
    {
        node->DrawLinks(zoom, origin);
    }
    
    if (const OutputRef& selectedOutput = m_selectedOutput.lock())
    {
        NodeWeakRef nodeWeakRef = GetNode(selectedOutput->parentUUID);
        Vec2f inPosition = nodeWeakRef.lock()->GetOutputPosition(selectedOutput->index, origin, zoom);
        ImGui::GetWindowDrawList()->AddLine(inPosition, mousePos, ImColor(255, 255, 255), 3);
    }
}

void NodeManager::SelectNode(const NodeRef& node)
{
    if (NodeRef selectedNode = m_selectedNode.lock())
        selectedNode->p_selected = false;
    
    m_selectedNode = node;

    if (NodeRef selectedNode = m_selectedNode.lock())
        selectedNode->p_selected = true;
}
