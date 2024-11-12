#include "NodeManager.h"

#include <fstream>
#include <ranges>
#include <unordered_set>

#include "LinkManager.h"
#include "Node.h"
#include "NodeTemplateHandler.h"
#include "Serializer.h"
#include "Actions/Action.h"
#include "Actions/ActionCreateLink.h"
#include "Actions/ActionDeleteNodesAndLinks.h"
#include "Actions/ActionMoveNodes.h"

std::string UserInputEnumToString(UserInputState userInputState)
{
    switch (userInputState)
    {
    case UserInputState::None:
        return "None";
    case UserInputState::DragNode:
        return "DragNode";
    case UserInputState::SelectingSquare:
        return "SelectingSquare";
    case UserInputState::CreateLink:
        return "CreateLink";
    case UserInputState::ClickNode:
        return "ClickNode";
    default:
        return "Unknown";
    }
}

NodeManager::NodeManager(MainWindow* window)
{
    m_linkManager = new LinkManager(this);

    auto node = NodeTemplateHandler::GetInstance()->CreateFromTemplate(0);
    
    AddNode(node);
}

NodeManager::~NodeManager()
{
    delete m_linkManager;
}

void NodeManager::AddNode(const NodeRef& node)
{
    if (m_nodes.contains(node->p_uuid))
    {
        std::cout << "Node with UUID " << node->p_uuid << " already exists\n";
        return;
    }
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
    ActionDeleteNodesAndLinks* action = new ActionDeleteNodesAndLinks(this, m_selectedNodes, m_linkManager->GetSelectedLinks());
    m_linkManager->DeleteSelectedLinks();
    
    for (int i = 0; i < m_selectedNodes.size(); i++)
    {
        if (!m_selectedNodes[i].lock()->p_allowInteraction)
            continue;
        RemoveNode(m_selectedNodes[i]);
        m_selectedNodes.erase(m_selectedNodes.begin() + i--);
    }
    ActionManager::AddAction(action);
}

void NodeManager::OnInputClicked(const NodeRef& node, bool altClicked, const uint32_t i)
{
    if (altClicked)
    {
        std::vector<LinkWeakRef> linkWithInput = m_linkManager->GetLinksWithInput(node->GetUUID(), i);
        
        ActionDeleteNodesAndLinks* action = new ActionDeleteNodesAndLinks(this, {}, linkWithInput);
        ActionManager::AddAction(action);
        
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
        LinkWeakRef linkWithOutput = m_linkManager->GetLinkWithOutput(node->GetUUID(), i);
        
        ActionDeleteNodesAndLinks* action = new ActionDeleteNodesAndLinks(this, {}, {linkWithOutput});
        ActionManager::AddAction(action);
        
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
        auto link = m_linkManager->AddLink(m_currentLink);
        ActionCreateLink* action = new ActionCreateLink(this, link.lock());
        ActionManager::AddAction(action);
        m_userInputState = UserInputState::None;
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
    else if (mouseClicked && !wasNodeClicked && node->IsSelected(mousePos, origin, zoom) && m_userInputState == UserInputState::None)
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

void NodeManager::UpdateDragging(float zoom, const Vec2f& origin, const Vec2f& mousePos)
{
    static float prevZoom = 1.f;
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
}

void NodeManager::UpdateSelectionSquare(float zoom, const Vec2f& origin, const Vec2f& mousePos)
{
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
}

void NodeManager::UpdateNodes(float zoom, const Vec2f& origin, const Vec2f& mousePos)
{
    UserInputState prevUserInputState = m_userInputState;
    bool mouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    bool wasNodeClicked = false;
    bool ctrlClick = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);

    if ((m_userInputState == UserInputState::DragNode || m_userInputState == UserInputState::SelectingSquare)
        && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        if (m_userInputState == UserInputState::DragNode)
        {
            ActionManager::UpdateLastAction();
        }
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
        m_linkManager->ClearSelectedLinks();
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
        ActionMoveNodes* action = new ActionMoveNodes(m_selectedNodes);
        ActionManager::AddAction(action);
    }
    else if (m_userInputState == UserInputState::None
        && ImGui::IsMouseDown(ImGuiMouseButton_Left)
        && ImGui::GetIO().MouseDelta != ImVec2(0.0f, 0.0f)
        && CurrentLinkIsNone())
    {
        SetUserInputState(UserInputState::SelectingSquare);
    }

    UpdateDragging(zoom, origin, mousePos);

    UpdateSelectionSquare(zoom, origin, mousePos);
    
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

void NodeManager::SaveToFile(const std::string& path) const
{
    CppSer::Serializer serializer(path);
    serializer.SetVersion("1.0");
    Serialize(serializer);
}

void NodeManager::LoadFromFile(const std::string& filePath)
{
    std::filesystem::path path(filePath);
    CppSer::Parser parser(path);
    if (!parser.IsFileOpen())
    {
        std::cout << "Failed to open file\n";
        return;
    }
    
    if (parser.GetVersion() != "1.0")
    {
        std::cout << "Invalid file version\n";
        return;
    }

    // Clean NodeManager
    Clean();
    
    Deserialize(parser);
}

void NodeManager::Serialize(CppSer::Serializer& serializer) const
{
    serializer << CppSer::Pair::BeginMap << "Nodes";
    serializer << CppSer::Pair::Key << "Node Count" << CppSer::Pair::Value << m_nodes.size();
    serializer << CppSer::Pair::BeginTab;
    for (const NodeRef& node : m_nodes | std::views::values)
    {
        node->Serialize(serializer);
    }
    serializer << CppSer::Pair::EndTab;
    serializer << CppSer::Pair::EndMap << "Nodes";

    m_linkManager->Serialize(serializer);
}

void NodeManager::SerializeSelectedNodes(CppSer::Serializer& serializer) const
{
    std::vector<NodeRef> nodesToSerialize;
    nodesToSerialize.reserve(m_selectedNodes.size());
    
    // Collect selected nodes with interaction enabled
    for (const NodeWeakRef& node : m_selectedNodes)
    {
        if (auto ref = node.lock(); ref && ref->p_allowInteraction)
        {
            nodesToSerialize.push_back(ref);
        }
    }

    // Serialize nodes
    serializer << CppSer::Pair::BeginMap << "Nodes";
    serializer << CppSer::Pair::Key << "Node Count" << CppSer::Pair::Value << nodesToSerialize.size();
    serializer << CppSer::Pair::BeginTab;
    for (const NodeRef& node : nodesToSerialize)
    {
        node->Serialize(serializer);
    }
    serializer << CppSer::Pair::EndTab;
    serializer << CppSer::Pair::EndMap << "Nodes";

    // Prepare a set of UUIDs for faster lookups
    std::unordered_set<UUID> nodeUUIDs;
    for (const NodeRef& node : nodesToSerialize)
    {
        nodeUUIDs.insert(node->GetUUID());
    }

    // Collect unique links
    std::vector<LinkRef> linkToSerialize;
    std::unordered_set<LinkRef> uniqueLinks;

    for (const NodeRef& node : nodesToSerialize)
    {
        auto links = node->GetLinks();
        for (const LinkWeakRef& link : links)
        {
            if (auto linkPtr = link.lock();
                nodeUUIDs.contains(linkPtr->fromNodeIndex) &&
                nodeUUIDs.contains(linkPtr->toNodeIndex) &&
                uniqueLinks.insert(linkPtr).second) // Insert only if unique
            {
                linkToSerialize.push_back(linkPtr);
            }
        }
    }

    // Serialize links
    LinkManager::Serialize(serializer, linkToSerialize);
}


void NodeManager::Deserialize(CppSer::Parser& parser)
{
    auto nodeTemplateHandler = NodeTemplateHandler::GetInstance();
    uint32_t nodeCount = parser["Node Count"].As<uint32_t>();
    for (uint32_t i = 0; i < nodeCount; i++)
    {
        parser.PushDepth();
        uint32_t templateID = parser["TemplateID"].As<uint32_t>();
        NodeRef node = nodeTemplateHandler->CreateFromTemplate(templateID);
        node->p_nodeManager = this;
        node->Deserialize(parser);
        AddNode(node);
    }

    m_linkManager->Deserialize(parser);
}

SerializedData NodeManager::DeserializeData(CppSer::Parser& parser)
{
    SerializedData data;
    auto nodeTemplateHandler = NodeTemplateHandler::GetInstance();
    uint32_t nodeCount = parser["Node Count"].As<uint32_t>();
    data.nodes.resize(nodeCount);
    for (uint32_t i = 0; i < nodeCount; i++)
    {
        parser.PushDepth();
        uint32_t templateID = parser["TemplateID"].As<uint32_t>();
        NodeRef node = nodeTemplateHandler->CreateFromTemplate(templateID);
        node->Deserialize(parser);
        data.nodes[i] = node;
    }

    LinkManager::Deserialize(parser, data.links);

    return data;
}


void NodeManager::Clean()
{
    m_linkManager->Clean();
    m_selectedNodes.clear();
    m_nodes.clear();
    m_currentLink = Link();
    m_userInputState = UserInputState::None;
    m_selectionSquare = SelectionSquare();
}
