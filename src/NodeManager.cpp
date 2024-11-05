#include "NodeManager.h"
#include <ranges>

#include "LinkManager.h"
#include "Node.h"

std::unique_ptr<NodeManager> NodeManager::m_instance;

NodeManager::NodeManager()
{
    m_linkManager = new LinkManager();

    NodeRef node = std::make_shared<Node>("Node");
    node->SetPosition(Vec2f(50, 50));
            
    node->AddInput("Input", Type::Float);
    node->AddInput("Input", Type::Vector2);
    node->AddInput("Input", Type::Vector3);
    node->AddOutput("Output", Type::Float);
    node->AddOutput("Output", Type::Vector2);
    node->AddOutput("Output", Type::Vector3);
            
    AddNode(node);

    NodeRef node2 = std::make_shared<Node>("Node");
    node2->SetPosition(Vec2f(500, 50));
            
    node2->AddInput("Input", Type::Float);
    node2->AddInput("Input", Type::Vector2);
    node2->AddInput("Input", Type::Vector3);
    node2->AddOutput("Output", Type::Float);
    node2->AddOutput("Output", Type::Vector2);
    node2->AddOutput("Output", Type::Vector3);
            
    AddNode(node2);

    // m_linkManager->CreateLink(node, 0, node2, 0);
    // m_linkManager->CreateLink(node, 0, node2, 1);
    // m_linkManager->CreateLink(node, 0, node2, 2);
}

NodeManager::~NodeManager()
{
    delete m_linkManager;
}

void NodeManager::AddNode(const NodeRef& node)
{
    m_nodes[node->p_uuid] = node;
    node->p_nodeManager = this;
}

void NodeManager::RemoveNode(const UUID& uuid)
{
    m_nodes.erase(uuid);
}

void NodeManager::RemoveNode(const NodeWeakRef& weak)
{
    NodeRef node = weak.lock();
    m_linkManager->RemoveLinks(node);
    RemoveNode(node->GetUUID());
}

void NodeManager::UpdateDelete()
{
    const bool deleteClicked = ImGui::IsKeyPressed(ImGuiKey_Delete);
    if (!deleteClicked)
        return;
    if (m_selectedLink.lock())
    {
        m_linkManager->RemoveLink(m_selectedLink);
    }
    if (m_selectedNode.lock())
    {
        RemoveNode(m_selectedNode);
    }
}

void NodeManager::OnInputClicked(const NodeRef& node, bool altClicked, size_t i)
{
    if (altClicked)
    {
        m_linkManager->RemoveLink(node->GetInput(i));
    }
    else
    {
        m_currentLink.toNodeIndex = node->p_uuid;
        m_currentLink.toInputIndex = i;

        if (m_currentLink.fromNodeIndex != -1 &&!m_linkManager->CanCreateLink(m_currentLink))
        {
            m_currentLink.toNodeIndex = -1;
            m_currentLink.toInputIndex = -1;
        }
    }
}

void NodeManager::OnOutputClicked(const NodeRef& node, bool altClicked, size_t i)
{
    if (altClicked)
    {
        m_linkManager->RemoveLinks(node->GetOutput(i));
    }
    else
    {
        m_currentLink.fromNodeIndex = node->p_uuid;
        m_currentLink.fromOutputIndex = i;
        
        if (m_currentLink.toNodeIndex != -1 && !m_linkManager->CanCreateLink(m_currentLink))
        {
            m_currentLink.fromNodeIndex = -1;
            m_currentLink.fromOutputIndex = -1;
        }
    }
}

void NodeManager::UpdateInOut(float zoom, const Vec2f& origin, const Vec2f& mousePos, bool mouseClicked, const NodeRef& node)
{
    if (!mouseClicked)
        return;
    
    bool altClicked = ImGui::IsKeyDown(ImGuiKey_LeftAlt);
    if (m_currentLink.toNodeIndex == -1 || altClicked)
    {
        for (size_t i = 0; i < node->p_inputs.size(); i++)
        {
            Vec2f circlePos = node->GetInputPosition(i);
            if (node->IsPointHoverCircle(mousePos, circlePos, origin, zoom, i))
            {
                OnInputClicked(node, altClicked, i);
                return;
            }
        }
    }

    if (m_currentLink.fromNodeIndex == -1 || altClicked)
    {
        for (size_t i = 0; i < node->p_outputs.size(); i++)
        {
            Vec2f circlePos = node->GetOutputPosition(i);
            if (node->IsPointHoverCircle(mousePos, circlePos, origin, zoom, i))
            {
                OnOutputClicked(node, altClicked, i);
                return;
            }
        }
    }
}

void NodeManager::UpdateCurrentLink()
{
    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        m_currentLink = Link();
    }
        
    if (m_currentLink.fromNodeIndex != -1 && m_currentLink.toNodeIndex != -1) // Is Linked
    {
        m_linkManager->AddLink(m_currentLink);
        m_currentLink = Link();
    }
}

void NodeManager::UpdateNodes(float zoom, const Vec2f& origin, const Vec2f& mousePos)
{
    m_zoom = zoom;
    m_origin = origin;
    
    bool mouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    NodeRef selectedNode = m_selectedNode.lock();
    bool wasNodeClicked = false;
    bool wasInputClicked = false;

    if (mouseClicked)
    {
        m_selectedLink = m_linkManager->GetLinkClicked(zoom, origin, mousePos).lock();
    }
    
    for (NodeRef& node : m_nodes | std::views::values)
    {
        bool alreadyOneSelected = selectedNode != m_selectedNode.lock();

        UpdateInOut(zoom, origin, mousePos, mouseClicked, node);
        
        UpdateCurrentLink();

        if (mouseClicked && !alreadyOneSelected && node->IsPointHoverNode(mousePos, origin, zoom))
        {
            SelectNode(node);
            
            m_firstClickOffset = mousePos;
            m_defaultPosition = node->p_position;

            wasNodeClicked = true;
        }
    }
    
    if (mouseClicked && !wasNodeClicked && !wasInputClicked)
    {
        SelectNode(nullptr);
    }

    bool isAlmostLinked = m_currentLink.fromNodeIndex != -1 || m_currentLink.toNodeIndex != -1;
    NodeRef currentSelectedNode = m_selectedNode.lock();
    // Move selected node
    if (!isAlmostLinked && currentSelectedNode && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        Vec2f newPosition = m_defaultPosition + (mousePos - m_firstClickOffset) / zoom;
        currentSelectedNode->SetPosition(newPosition);
    }

    UpdateDelete();
}

void NodeManager::DrawNodes(float zoom, const Vec2f& origin, const Vec2f& mousePos) const
{
    for (const NodeRef& node : m_nodes | std::views::values)
    {
        node->Draw(zoom, origin);
    }
    
    if (IsAlmostLinked())
    {
        Vec2f inPosition = mousePos;
        Vec2f outPosition = mousePos;

        if (m_currentLink.fromNodeIndex != -1)
        {
            inPosition = GetNode(m_currentLink.fromNodeIndex).lock()->GetOutputPosition(m_currentLink.fromOutputIndex, origin, zoom);
        }
        else if (m_currentLink.toNodeIndex != -1)
        {
            outPosition = GetNode(m_currentLink.toNodeIndex).lock()->GetInputPosition(m_currentLink.toInputIndex, origin, zoom);
        }
        
        ImGui::GetWindowDrawList()->AddLine(inPosition, outPosition, ImColor(255, 255, 255), 3 * zoom);
    }

    m_linkManager->DrawLinks(zoom, origin);
}

void NodeManager::SelectNode(const NodeRef& node)
{
    if (NodeRef selectedNode = m_selectedNode.lock())
        selectedNode->p_selected = false;
    
    m_selectedNode = node;

    if (NodeRef selectedNode = m_selectedNode.lock())
        selectedNode->p_selected = true;
}

LinkWeakRef NodeManager::GetLinkWithOutput(const UUID& uuid, uint32_t index) const
{
    return m_linkManager->GetLinkWithOutput(uuid, index);
}

bool NodeManager::IsAlmostLinked() const
{
    return m_currentLink.fromNodeIndex != -1 || m_currentLink.toNodeIndex != -1;
}
