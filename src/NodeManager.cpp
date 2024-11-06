#include "NodeManager.h"
#include <ranges>

#include "LinkManager.h"
#include "Node.h"

std::unique_ptr<NodeManager> NodeManager::m_instance;

NodeManager::NodeManager()
{
    m_linkManager = new LinkManager();

    NodeRef node = std::make_shared<Node>("Material");
    node->SetTopColor(IM_COL32(156, 122, 72, 255));
    node->SetPosition(Vec2f(50, 50));
            
    node->AddInput("Base Color", Type::Vector3);
    node->AddInput("Metallic", Type::Float);
    node->AddInput("Specular", Type::Float);
    node->AddInput("Roughness", Type::Float);
    
    AddNode(node);       
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
    m_linkManager->DeleteSelectedLinks();
    
    for (auto& selectedNode : m_selectedNodes)
    {
        RemoveNode(selectedNode);
    }
}

void NodeManager::OnInputClicked(const NodeRef& node, bool altClicked, const uint32_t i)
{
    if (altClicked)
    {
        m_linkManager->RemoveLink(node->GetInput(i));
    }
    else
    {
        m_currentLink.toNodeIndex = node->p_uuid;
        m_currentLink.toInputIndex = i;

        if (m_currentLink.fromNodeIndex != UUID_NULL &&!m_linkManager->CanCreateLink(m_currentLink))
        {
            m_currentLink.toNodeIndex = UUID_NULL;
            m_currentLink.toInputIndex = UUID_NULL;
        }
    }
}

void NodeManager::OnOutputClicked(const NodeRef& node, bool altClicked, uint32_t i)
{
    if (altClicked)
    {
        m_linkManager->RemoveLinks(node->GetOutput(i));
    }
    else
    {
        m_currentLink.fromNodeIndex = node->p_uuid;
        m_currentLink.fromOutputIndex = i;
        
        if (m_currentLink.toNodeIndex != UUID_NULL && !m_linkManager->CanCreateLink(m_currentLink))
        {
            m_currentLink.fromNodeIndex = UUID_NULL;
            m_currentLink.fromOutputIndex = UUID_NULL;
        }
    }
}

void NodeManager::UpdateInOut(float zoom, const Vec2f& origin, const Vec2f& mousePos, bool mouseClicked, const NodeRef& node)
{
    if (!mouseClicked)
        return;
    
    bool altClicked = ImGui::IsKeyDown(ImGuiKey_LeftAlt);
    if (m_currentLink.toNodeIndex == UUID_NULL || altClicked)
    {
        for (uint32_t i = 0; i < node->p_inputs.size(); i++)
        {
            Vec2f circlePos = node->GetInputPosition(i, origin, zoom);
            if (node->IsPointHoverCircle(mousePos, circlePos, origin, zoom, i))
            {
                OnInputClicked(node, altClicked, i);
                return;
            }
        }
    }

    if (m_currentLink.fromNodeIndex == UUID_NULL || altClicked)
    {
        for (uint32_t i = 0; i < node->p_outputs.size(); i++)
        {
            Vec2f circlePos = node->GetOutputPosition(i, origin, zoom);
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
        
    if (m_currentLink.fromNodeIndex != UUID_NULL && m_currentLink.toNodeIndex != UUID_NULL) // Is Linked
    {
        m_linkManager->AddLink(m_currentLink);
        m_currentLink = Link();
    }
}

void NodeManager::UpdateNodes(float zoom, const Vec2f& origin, const Vec2f& mousePos)
{    
    bool mouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    bool wasNodeClicked = false;
    bool wasInputClicked = false;
    bool alreadyOneSelected = false;
    bool ctrlClick = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);
    
    for (NodeRef& node : m_nodes | std::views::values)
    {        
        UpdateInOut(zoom, origin, mousePos, mouseClicked, node);
        UpdateCurrentLink();

        if (mouseClicked && !alreadyOneSelected && node->IsSelected(mousePos, origin, zoom))
        {
            if (ctrlClick)
            {
                if (!node->p_selected)
                {
                    AddSelectedNode(node);
                }
                else
                {
                    RemoveSelectedNode(node);
                }
            }
            else if (!node->p_selected)
            {
                SelectNode(node);
            }
            
            for (auto& selectedNode : m_selectedNodes)
            {
                // Calculate offset accounting for zoom and origin
                selectedNode.lock()->p_clickOffset = (selectedNode.lock()->p_position - (mousePos / zoom + origin));
            }

            wasNodeClicked = true;
            alreadyOneSelected = true;
        }
    }

    // Cancel when escape pressed
    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        m_currentLink = Link();
    }
    
    if (mouseClicked && !wasNodeClicked && !wasInputClicked)
    {
        SelectNode(nullptr);
    }

    // Move selected nodes with correct position adjustment
    if (!IsAlmostLinked() && !m_selectedNodes.empty()
        && ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::GetIO().MouseDelta != ImVec2(0.0f, 0.0f))
    {
        for (const auto& m_selectedNode : m_selectedNodes)
        {
            Node* currentSelectedNode = m_selectedNode.lock().get();
            Vec2f newPosition = (mousePos / zoom + origin) + currentSelectedNode->p_clickOffset;
            m_selectedNode.lock()->SetPosition(newPosition);
        }
    }

    UpdateDelete();
}




void NodeManager::DrawNodes(float zoom, const Vec2f& origin, const Vec2f& mousePos) const
{
    for (const NodeRef& node : m_nodes | std::views::values)
    {
        
        if (!node->IsNodeVisible(origin, zoom))
            continue;
        node->Draw(zoom, origin);
    }
    
    if (IsAlmostLinked())
    {
        Vec2f inPosition = mousePos;
        Vec2f outPosition = mousePos;

        if (m_currentLink.fromNodeIndex != UUID_NULL)
        {
            inPosition = GetNode(m_currentLink.fromNodeIndex).lock()->GetOutputPosition(m_currentLink.fromOutputIndex, origin, zoom);
        }
        else if (m_currentLink.toNodeIndex != UUID_NULL)
        {
            outPosition = GetNode(m_currentLink.toNodeIndex).lock()->GetInputPosition(m_currentLink.toInputIndex, origin, zoom);
        }
        
        ImGui::GetWindowDrawList()->AddLine(inPosition, outPosition, ImColor(255, 255, 255), 3 * zoom);
    }

    m_linkManager->DrawLinks(zoom, origin);
}

void NodeManager::SelectNode(const NodeRef& node)
{
    ClearSelectedNodes();
    AddSelectedNode(node);
}

void NodeManager::AddSelectedNode(const NodeRef& node)
{
    if (!node)
        return;
    m_selectedNodes.push_back(node);
    node->p_selected = true;
}

void NodeManager::RemoveSelectedNode(const NodeWeakRef& node)
{
    for (auto it = m_selectedNodes.begin(); it != m_selectedNodes.end(); it++)
    {
        if ((*it).lock() == node.lock())
        {
            it->lock()->p_selected = false;
            m_selectedNodes.erase(it);
            return;
        }
    }
}

void NodeManager::ClearSelectedNodes()
{
    for (NodeRef& node : m_nodes | std::views::values)
    {
        node->p_selected = false;
    }
    m_selectedNodes.clear();
}

LinkWeakRef NodeManager::GetLinkWithOutput(const UUID& uuid, const uint32_t index) const
{
    return m_linkManager->GetLinkWithOutput(uuid, index);
}

bool NodeManager::IsAlmostLinked() const
{
    return m_currentLink.fromNodeIndex != UUID_NULL || m_currentLink.toNodeIndex != UUID_NULL;
}
