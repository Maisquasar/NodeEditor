#pragma once
#include <functional>
#include <galaxymath/Maths.h>
#include <memory>
#include <unordered_map>

#include "Event.h"
#include "LinkManager.h"
#include "UUID.h"

#include "Node.h"


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

class NodeManager
{
public:
    NodeManager();
    ~NodeManager();

    static NodeManager* Get() { return m_instance.get(); }
    static void Create() { m_instance = std::make_unique<NodeManager>(); }
    
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
    void UpdateDelete();

    void DrawNodes(float zoom, const Vec2f& origin, const Vec2f& mousePos) const;
    void UpdateNodes(float zoom, const Vec2f& origin, const Vec2f& mousePos);

    void SelectNode(const NodeRef& node);
    void AddSelectedNode(const NodeRef& node);
    void RemoveSelectedNode(const NodeWeakRef& node);
    void ClearSelectedNodes();
    
    LinkManager* GetLinkManager() const { return m_linkManager; }
    NodeWeakRef GetNode(const UUID& uuid) const{ return m_nodes.at(uuid); }
    InputWeakRef GetInput(const UUID& uuid, const uint32_t index) { return m_nodes[uuid]->GetInput(index); }
    OutputWeakRef GetOutput(const UUID& uuid, const uint32_t index) { return m_nodes[uuid]->GetOutput(index); }
    LinkWeakRef GetLinkWithOutput(const UUID& uuid, uint32_t index) const;

    // Link
    bool CurrentLinkIsAlmostLinked() const;
    bool CurrentLinkIsNone() const;

    void SetUserInputState(const UserInputState& state) { m_userInputState = state; }
    UserInputState GetUserInputState() const { return m_userInputState; }

    SelectionSquare GetSelectionSquare() const { return m_selectionSquare; }

    Utils::EventWithID<> EOnDrawEvent;
private:
    static std::unique_ptr<NodeManager> m_instance;
    
    LinkManager* m_linkManager = nullptr;
    
    NodeList m_nodes;

    Link m_currentLink; // The link when creating a new link

    std::vector<NodeWeakRef> m_selectedNodes;

    Vec2f m_onClickPos;

    SelectionSquare m_selectionSquare;

    UserInputState m_userInputState = UserInputState::None;
};
