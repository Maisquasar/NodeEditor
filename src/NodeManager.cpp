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
    node->SetPosition(Vec2f(0, 0));
            
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

void NodeManager::UpdateInputOutputClick(float zoom, const Vec2f& origin, const Vec2f& mousePos, bool mouseClicked, const NodeRef& node)
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

void NodeManager::UpdateNodeSelection(NodeRef node, float zoom, const Vec2f& origin, const Vec2f& mousePos,
                                      bool mouseClicked, bool ctrlDown, bool& wasNodeClicked)
{
    if (m_selectionSquare.shouldDraw)
    {
        if (node->IsSelected(m_selectionSquare.min, m_selectionSquare.max, origin, zoom))
        {
            if (!node->p_selected)
            {
                AddSelectedNode(node);
            }
        }
    }
    else if (mouseClicked && !wasNodeClicked && node->IsSelected(mousePos, origin, zoom))
    {
        SetUserInputState(UserInputState::ClickNode);
        if (ctrlDown)
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
            
        m_onClickPos = mousePos;
        for (auto& selectedNode : m_selectedNodes)
        {
            auto selectedNodeLock = selectedNode.lock();
            Vec2f posOnScreen = ToScreen(selectedNode.lock()->p_position, zoom, origin);
            selectedNodeLock->p_positionOnClick = posOnScreen ;
        }

        wasNodeClicked = true;
    }
}

void NodeManager::UpdateNodes(float zoom, const Vec2f& origin, const Vec2f& mousePos)
{
    bool mouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    bool wasNodeClicked = false;
    bool ctrlClick = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);
    static float prevZoom = 1.f;

    if ((m_userInputState == UserInputState::DragNode || m_userInputState == UserInputState::SelectingSquare)
        && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        SetUserInputState(UserInputState::None);
    }
    else if (m_userInputState == UserInputState::SelectingSquare)
    {
        ClearSelectedNodes();
    }

    for (NodeRef& node : m_nodes | std::views::values)
    {
        node->p_isVisible = node->IsNodeVisible(origin, zoom);

        if (!node->p_isVisible)
            continue;
        
        UpdateInputOutputClick(zoom, origin, mousePos, mouseClicked, node);

        UpdateNodeSelection(node, zoom, origin, mousePos, mouseClicked, ctrlClick, wasNodeClicked);
    }

    m_linkManager->UpdateLinkSelection(origin, zoom);
    
    UpdateCurrentLink();

    if (m_userInputState == UserInputState::ClickNode && CurrentLinkIsAlmostLinked())
    {
        RemoveSelectedNode(m_selectedNodes.back());
        SetUserInputState(UserInputState::CreateLink);
    }

    // Cancel when escape pressed
    if (m_userInputState == UserInputState::CreateLink && ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        SetUserInputState(UserInputState::None);
        m_currentLink = Link();
    }
    
    if (mouseClicked && !wasNodeClicked)
    {
        SelectNode(nullptr);
    }
    
    if (mouseClicked)
    {
        Vec2f localMousePos = ToGrid(mousePos, zoom, origin);
        m_selectionSquare.mousePosOnStart = localMousePos;
    }

    // Update States
    if (m_userInputState == UserInputState::ClickNode
        && ImGui::IsMouseDown(ImGuiMouseButton_Left)
        && !CurrentLinkIsAlmostLinked())
    {
        SetUserInputState(UserInputState::DragNode);
    }
    else if (m_userInputState == UserInputState::None
        && ImGui::IsMouseDown(ImGuiMouseButton_Left)
        && ImGui::GetIO().MouseDelta != ImVec2(0.0f, 0.0f)
        && CurrentLinkIsNone())
    {
        SetUserInputState(UserInputState::SelectingSquare);
    }

    // Update dragging
    if (m_userInputState == UserInputState::DragNode)
    {
        bool changePosition = false;
        if (prevZoom != zoom)
        {
            changePosition = true;
            prevZoom = zoom;
        }
            
        // Move nodes
        for (const auto& selectedNode : m_selectedNodes)
        {
            if (changePosition)
            {
                Vec2f posOnScreen = ToScreen(selectedNode.lock()->p_position, zoom, origin);
                selectedNode.lock()->p_positionOnClick = posOnScreen ;
                m_onClickPos = mousePos;
            }
            Node* currentSelectedNode = selectedNode.lock().get();
            Vec2f offset = m_onClickPos - currentSelectedNode->p_positionOnClick;
            Vec2f newPosition = ToGrid(mousePos - offset, zoom, origin) ;
            
            currentSelectedNode->SetPosition(newPosition);
        }
    }

    // Update selection square rendering
    if (m_userInputState == UserInputState::SelectingSquare)
    {
        m_selectionSquare.min = m_selectionSquare.mousePosOnStart * zoom + origin;
        m_selectionSquare.max = mousePos;
        m_selectionSquare.shouldDraw = true;
    }
    else
    {
        m_selectionSquare.shouldDraw = false;
    }
    
    
    UpdateDelete();
}

void NodeManager::DrawNodes(float zoom, const Vec2f& origin, const Vec2f& mousePos) const
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    for (const NodeRef& node : m_nodes | std::views::values)
    {
        if (!node->p_isVisible)
            continue;
        node->Draw(zoom, origin);
    }
    
    if (m_userInputState == UserInputState::CreateLink)
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

        drawList->AddLine(inPosition, outPosition, IM_COL32(255, 255, 255, 255), 3 * zoom);
    }

    m_linkManager->DrawLinks(zoom, origin);
    
    if (m_selectionSquare.shouldDraw)
    {
        drawList->AddRectFilled(m_selectionSquare.min, m_selectionSquare.max, IM_COL32(36, 91, 130, 155));
        drawList->AddRect(m_selectionSquare.min, m_selectionSquare.max, IM_COL32(255, 255, 255, 255));
    }
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
        if (it->lock() == node.lock())
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

bool NodeManager::CurrentLinkIsAlmostLinked() const
{
    return m_currentLink.fromNodeIndex != UUID_NULL || m_currentLink.toNodeIndex != UUID_NULL;
}

bool NodeManager::CurrentLinkIsNone() const
{
    return m_currentLink.fromNodeIndex == UUID_NULL && m_currentLink.toNodeIndex == UUID_NULL;
}
