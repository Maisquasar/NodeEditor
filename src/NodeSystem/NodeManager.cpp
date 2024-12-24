#include "NodeSystem/NodeManager.h"
#include "NodeSystem/LinkManager.h"
#include "NodeSystem/Node.h"
#include "NodeSystem/NodeTemplateHandler.h"

#include <fstream>
#include <ranges>
#include <unordered_set>

#include "NodeWindow.h"
#include "Serializer.h"
#include "Type.h"
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
    case UserInputState::CreateNode:
        return "CreateNode";
    case UserInputState::ClickNode:
        return "ClickNode";
    default:
        return "Unknown";
    }
}

NodeManager::NodeManager(NodeWindow* window) : m_parent(window)
{
    m_linkManager = new LinkManager(this);

    AddNode(NodeTemplateHandler::CreateFromTemplateName("Material"));
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
    auto node = m_nodes.find(uuid);
    if (node == m_nodes.end())
    {
        std::cout << "Node with UUID " << uuid << " does not exist\n";
        return;
    }
    node->second->OnRemove();
    m_nodes.erase(uuid);
}

void NodeManager::RemoveNode(const NodeWeak& weak)
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
    auto action = std::make_shared<ActionDeleteNodesAndLinks>(this, m_selectedNodes, m_linkManager->GetSelectedLinks());
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

        SetUserInputState(UserInputState::Busy);

        std::vector<NodeWeak> nodes = {};
        auto action = std::make_shared<ActionDeleteNodesAndLinks>(this, nodes, linkWithInput);
        ActionManager::AddAction(action);
        
        m_linkManager->RemoveLink(node->GetInput(i));
    }
    else
    {
        m_currentLink.toNodeIndex = node->p_uuid;
        m_currentLink.toInputIndex = i;

        if (m_currentLink.fromNodeIndex != UUID_NULL && !m_linkManager->CanCreateLink(m_currentLink))
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
        std::vector<LinkWeakRef> links = m_linkManager->GetLinksWithOutput(node->GetOutput(i));
        std::vector<NodeWeak> nodes = {};
        
        auto action = std::make_shared<ActionDeleteNodesAndLinks>(this, nodes, links);
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
    bool altClicked = ImGui::IsKeyDown(ImGuiKey_LeftAlt);
    if (m_currentLink.toNodeIndex == UUID_NULL || altClicked)
    {
        for (uint32_t i = 0; i < node->p_inputs.size(); i++)
        {
            Vec2f circlePos = node->GetInputPosition(i, origin, zoom);
            if (node->IsPointHoverCircle(mousePos, circlePos, origin, zoom, i))
            {
                SetHoveredStream(node->p_inputs[i]);
                if (GetUserInputState() == UserInputState::None && mouseClicked)
                {
                    OnInputClicked(node, altClicked, i);
                    SetUserInputState(UserInputState::CreateLink);
                }
                return;
            }
        }
    }

    if (m_currentLink.fromNodeIndex == UUID_NULL || altClicked)
    {
        for (uint32_t i = 0; i < node->p_outputs.size(); i++)
        {
            Vec2f circlePos = node->GetOutputPosition(i, origin, zoom);
            if (Node::IsPointHoverCircle(mousePos, circlePos, origin, zoom, i))
            {
                SetHoveredStream(node->p_outputs[i]);
                if (GetUserInputState() == UserInputState::None && mouseClicked)
                {
                    OnOutputClicked(node, altClicked, i);
                    SetUserInputState(UserInputState::CreateLink);
                }
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
        auto action =std::make_shared<ActionCreateLink>(this, link.lock());
        ActionManager::AddAction(action);
        ClearCurrentLink();
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
    else if (mouseClicked && !wasNodeClicked && node->IsSelected(mousePos, origin, zoom)
        && (m_userInputState == UserInputState::None || m_userInputState == UserInputState::ClickNode))
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
    if (m_userInputState != UserInputState::DragNode)
        return;
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
    if (!m_isGridHovered && !m_firstFrame)
        return;
    SetHoveredStream({});
    m_firstFrame = false;
    UserInputState prevUserInputState = m_userInputState;
    bool mouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    bool wasNodeClicked = false;
    bool ctrlClick = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);
    size_t selectedLinkSize = m_linkManager->GetSelectedLinks().size();

    if ((m_userInputState == UserInputState::DragNode || m_userInputState == UserInputState::SelectingSquare || m_userInputState == UserInputState::ClickNode)
        && ImGui::IsMouseReleased(ImGuiMouseButton_Left) || m_userInputState == UserInputState::Busy)
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
        if (!node->p_computed)
        {
            node->p_computed = true;
            node->ComputeNodeSize();
        }
        node->p_isVisible = node->IsNodeVisible(origin, zoom);

        if (!node->p_isVisible)
            continue;


        if (m_userInputState == UserInputState::None && node->p_allowPreview)
        {
            if (node->p_previewHovered = node->IsPreviewHovered(mousePos, origin, zoom))
            {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    node->OpenPreview(!node->p_preview);
                    break;
                }
            }
        }
        
        UpdateInputOutputClick(zoom, origin, mousePos, mouseClicked, node);

        UpdateNodeSelection(node, zoom, origin, mousePos, mouseClicked, ctrlClick, wasNodeClicked);

        node->Update();
    }

    m_linkManager->UpdateLinkSelection(origin, zoom);

    if (m_userInputState == UserInputState::CreateLink && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !m_parent->IsContextMenuOpen())
    {
        if (m_hoveredStream.lock())
        {
            if (auto input = std::dynamic_pointer_cast<Input>( m_hoveredStream.lock()))
            {
                m_currentLink.toNodeIndex = input->parentUUID;
                m_currentLink.toInputIndex = input->index;
        
                if (m_currentLink.toNodeIndex != UUID_NULL && !m_linkManager->CanCreateLink(m_currentLink))
                {
                    ClearCurrentLink();
                }
                else
                {
                    InputRef inputRef = GetInput(m_currentLink.toNodeIndex, m_currentLink.toInputIndex).lock();
                    assert(inputRef);
                    m_linkManager->RemoveLink(inputRef);
                }
            }
            else if (auto output = std::dynamic_pointer_cast<Output>( m_hoveredStream.lock()))
            {
                m_currentLink.fromNodeIndex = output->parentUUID;
                m_currentLink.fromOutputIndex = output->index;
        
                if (m_currentLink.fromNodeIndex != UUID_NULL && !m_linkManager->CanCreateLink(m_currentLink))
                {
                    ClearCurrentLink();
                }
            }
        }
        else
        {
            m_parent->SetOpenContextMenu(true);
        }
    }
    
    UpdateCurrentLink();

    // Cancel when escape pressed
    if ((m_userInputState == UserInputState::CreateLink || m_userInputState == UserInputState::CreateNode) && ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        SetUserInputState(UserInputState::None);
        ClearCurrentLink();
        m_parent->SetOpenContextMenu(false);
    }
    
    if (mouseClicked && !wasNodeClicked && selectedLinkSize == m_linkManager->GetSelectedLinks().size())
    {
        // SetUserInputState(UserInputState::None);
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
        && !CurrentLinkIsAlmostLinked()
        && ImGui::GetIO().MouseDelta != ImVec2(0.0f, 0.0f))
    {
        SetUserInputState(UserInputState::DragNode);
        auto action = std::make_shared<ActionMoveNodes>(m_selectedNodes);
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

        if (GetMainWindow()->IsContextMenuOpen())
        {
            inPosition = GetMainWindow()->GetMousePosOnContext();
            outPosition = GetMainWindow()->GetMousePosOnContext();
        }

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

void NodeManager::RemoveSelectedNode(const NodeWeak& node)
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

NodeWeak NodeManager::GetNode(const UUID& uuid) const
{
    if (m_nodes.find(uuid) == m_nodes.end())
    {
        return {};
    }
    return m_nodes.at(uuid);
}

NodeWeak NodeManager::GetNodeWithTemplate(TemplateID templateID)
{
    for (auto& val : m_nodes | std::views::values)
    {
        if (val->p_templateID == templateID)
        {
            return val;
        }
    }
    return {};
}

NodeWeak NodeManager::GetNodeWithName(const std::string& name)
{
    for (auto& val : m_nodes | std::views::values)
    {
        if (val->p_name == name)
        {
            return val;
        }
    }
    return {};
}

std::vector<NodeWeak> NodeManager::GetNodeConnectedTo(const UUID& uuid) const
{
    std::vector<NodeWeak> outputs;
    NodeRef node = GetNode(uuid).lock();

    auto linkManager = GetLinkManager();

    for (int i = 0; i < node->p_inputs.size(); i++)
    {
        LinkRef val = linkManager->GetLinkLinkedToInput(uuid, i).lock();
        if (!val)
            continue;
        auto connectedNode = GetNode(val->fromNodeIndex);
        outputs.push_back(connectedNode);
        std::vector<NodeWeak> nodeConnectedTo = GetNodeConnectedTo(connectedNode.lock()->GetUUID());
        outputs.insert(outputs.begin(), nodeConnectedTo.begin(), nodeConnectedTo.end());
    }

    return outputs;
}

std::vector<LinkWeakRef> NodeManager::GetLinkWithOutput(const UUID& uuid, const uint32_t index) const
{
    return m_linkManager->GetLinksWithOutput(GetNode(uuid).lock()->GetOutput(index));
}

NodeWeak NodeManager::GetSelectedNode() const
{
    if (m_selectedNodes.empty())
        return {};
    return m_selectedNodes[0];
}

bool NodeManager::CurrentLinkIsAlmostLinked() const
{
    return m_currentLink.fromNodeIndex != UUID_NULL || m_currentLink.toNodeIndex != UUID_NULL;
}

bool NodeManager::CurrentLinkIsNone() const
{
    return m_currentLink.fromNodeIndex == UUID_NULL && m_currentLink.toNodeIndex == UUID_NULL;
}

void NodeManager::ClearCurrentLink()
{
    m_userInputState = UserInputState::None;
    m_currentLink = Link();
}

void NodeManager::SetUserInputState(const UserInputState& state)
{
#ifdef _DEBUG
    if (m_userInputState != state)
    {
        m_userInputState = state;
        std::cout << "Set User Input -> " << UserInputEnumToString(state) << "\n";
    }
#else
    m_userInputState = state;
#endif
}

void NodeManager::SaveToFile(const std::string& path)
{
    m_savePath = path;
    CppSer::Serializer serializer(path);
    serializer.SetVersion("1.0");
    Serialize(serializer);
}

void NodeManager::LoadFromFile(const std::string& filePath)
{
    std::filesystem::path path(filePath);
    m_savePath = filePath;
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
    m_firstFrame = true;

    m_parent->ShouldUpdateShader();
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
    for (const NodeWeak& node : m_selectedNodes)
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

    auto links = m_linkManager->GetLinks();
    for (const NodeRef& node : nodesToSerialize)
    {
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
    uint32_t nodeCount = parser["Node Count"].As<uint32_t>();
    for (uint32_t i = 0; i < nodeCount; i++)
    {
        parser.PushDepth();
        TemplateID templateID = parser["TemplateID"].As<uint64_t>();
        NodeRef node = NodeTemplateHandler::CreateFromTemplate(templateID);
        if (!node)
        {
            std::cout << "Failed to create node\n";
            continue;
        }
        node->p_nodeManager = this;
        node->Deserialize(parser);
        AddNode(node);
    }

    m_linkManager->Deserialize(parser);
}

SerializedData NodeManager::DeserializeData(CppSer::Parser& parser)
{
    SerializedData data;
    uint32_t nodeCount = parser["Node Count"].As<uint32_t>();
    data.nodes.resize(nodeCount);
    for (uint32_t i = 0; i < nodeCount; i++)
    {
        parser.PushDepth();
        TemplateID templateID = parser["TemplateID"].As<uint64_t>();
        NodeRef node = NodeTemplateHandler::CreateFromTemplate(templateID);
        node->p_nodeManager = this;
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

void NodeManager::SetHoveredStream(const Weak<Stream>& stream)
{
    if (auto prev = m_hoveredStream.lock())
    {
        prev->isHovered = false;
    }
    m_hoveredStream = stream;
    if (const auto curr = m_hoveredStream.lock())
    {
        curr->isHovered = true;
    }
}
