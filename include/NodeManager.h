#pragma once
#include <functional>
#include <galaxymath/Maths.h>
#include <memory>
#include <unordered_map>

#include "Event.h"
#include "LinkManager.h"
#include "UUID.h"

#include "Node.h"


class MainWindow;
class LinkManager;
using NodeList = std::pmr::unordered_map<UUID, NodeRef>;
struct SelectionSquare
{
    Vec2f mousePosOnStart;
    Vec2f min, max;
    bool shouldDraw = false;
};

enum class UserInputState
{
    None,
    CreateLink,
    ClickNode,
    DragNode,
    SelectingSquare,
};
std::string UserInputEnumToString(UserInputState userInputState);

struct SerializedData
{
    std::vector<NodeRef> nodes;
    std::vector<LinkRef> links;
};

class NodeManager
{
public:
    NodeManager(MainWindow* window);
    ~NodeManager();
    
    void AddNode(const NodeRef& node);
    void RemoveNode(const UUID& uuid);
    void RemoveNode(const NodeWeakRef& weak);

    static Vec2f ToScreen(const Vec2f& worldPos, float zoom, const Vec2f& origin) { return worldPos * zoom + origin; }
    static Vec2f ToGrid(const Vec2f& screenPos, float zoom, const Vec2f& origin) { return (screenPos - origin) / zoom; }
    
    void OnInputClicked(const NodeRef& node, bool altClicked, uint32_t i);
    void OnOutputClicked(const NodeRef& node, bool altClicked, uint32_t i);
    
    void UpdateInputOutputClick(float zoom, const Vec2f& origin, const Vec2f& mousePos, bool mouseClicked, const NodeRef& node);
    void UpdateCurrentLink();
    void UpdateNodeSelection(NodeRef node, float zoom, const Vec2f& origin, const Vec2f& mousePos, bool mouseClicked, bool ctrlDown, bool& wasNodeClicked);
    void UpdateDragging(float zoom, const Vec2f& origin, const Vec2f& mousePos);
    void UpdateSelectionSquare(float zoom, const Vec2f& origin, const Vec2f& mousePos);
    void UpdateDelete();

    void DrawNodes(float zoom, const Vec2f& origin, const Vec2f& mousePos) const;
    void UpdateNodes(float zoom, const Vec2f& origin, const Vec2f& mousePos);

    void SelectNode(const NodeRef& node);
    void AddSelectedNode(const NodeRef& node);
    void RemoveSelectedNode(const NodeWeakRef& node);
    void ClearSelectedNodes();
    
    LinkManager* GetLinkManager() const { return m_linkManager; }
    NodeWeakRef GetNode(const UUID& uuid) const{ return m_nodes.at(uuid); }
    NodeWeakRef GetNodeWithTemplate(TemplateID templateID);
    std::vector<NodeWeakRef> GetNodeConnectedTo(const UUID& uuid) const;
    InputWeakRef GetInput(const UUID& uuid, const uint32_t index) { return m_nodes[uuid]->GetInput(index); }
    OutputWeakRef GetOutput(const UUID& uuid, const uint32_t index) { return m_nodes[uuid]->GetOutput(index); }
    LinkWeakRef GetLinkWithOutput(const UUID& uuid, uint32_t index) const;
    NodeWeakRef GetSelectedNode() const;
    Link& GetCurrentLink() {return m_currentLink;}

    // Link
    bool CurrentLinkIsAlmostLinked() const;
    bool CurrentLinkIsNone() const;

    void SetUserInputState(const UserInputState& state) { m_userInputState = state; }
    UserInputState GetUserInputState() const { return m_userInputState; }

    SelectionSquare GetSelectionSquare() const { return m_selectionSquare; }

    void SaveToFile(const std::string& path) const;
    void LoadFromFile(const std::string& path);
    
    void Serialize(CppSer::Serializer& serializer) const;
    void SerializeSelectedNodes(CppSer::Serializer& serializer) const;

    void Deserialize(CppSer::Parser& parser);
    static SerializedData DeserializeData(CppSer::Parser& parser);

    void Paste();
    
    void Clean();

    Utils::EventWithID<> EOnDrawEvent;

private:
    friend class MainWindow;
    friend class ShaderMaker;
    MainWindow* m_parent;
    LinkManager* m_linkManager = nullptr;
    NodeList m_nodes;
    
    Link m_currentLink; // The link when creating a new link
    std::vector<NodeWeakRef> m_selectedNodes;

    UserInputState m_userInputState = UserInputState::None;

    bool m_firstFrame = true;
    
    bool m_isGridHovered = true;
    Vec2f m_onClickPos;
    SelectionSquare m_selectionSquare;
};
