#pragma once
#include <filesystem>
#include <functional>
#include <galaxymath/Maths.h>
#include <memory>
#include <unordered_map>

#include "Event.h"
#include "LinkManager.h"
#include "UUID.h"

#include "Node.h"
#include "Type.h"


class NodeWindow;
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
    Busy,
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
    NodeManager(NodeWindow* window);
    ~NodeManager();
    
    void AddNode(const NodeRef& node);
    void RemoveNode(const UUID& uuid);
    void RemoveNode(const NodeWeak& weak);

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
    void RemoveSelectedNode(const NodeWeak& node);
    void ClearSelectedNodes();
    
    LinkManager* GetLinkManager() const { return m_linkManager; }
    NodeWeak GetNode(const UUID& uuid) const;
    NodeWeak GetNodeWithTemplate(TemplateID templateID);
    NodeWeak GetNodeWithName(const std::string& name);
    std::vector<NodeWeak> GetNodeConnectedTo(const UUID& uuid) const;
    bool NodeExists(const UUID& uuid) const { return m_nodes.find(uuid) != m_nodes.end(); }
    InputWeak GetInput(const UUID& uuid, const uint32_t index) { return m_nodes[uuid]->GetInput(index); }
    OutputWeak GetOutput(const UUID& uuid, const uint32_t index) { return m_nodes[uuid]->GetOutput(index); }
    std::vector<LinkWeakRef> GetLinkWithOutput(const UUID& uuid, uint32_t index) const;
    NodeWeak GetSelectedNode() const;
    Link& GetCurrentLink() {return m_currentLink;}
    std::filesystem::path GetFilePath() const {return m_savePath;}
    NodeWindow* GetMainWindow() const { return m_parent; }
    StreamWeak GetHoveredStream() const {return m_hoveredStream;}

    // Link
    bool CurrentLinkIsAlmostLinked() const;
    bool CurrentLinkIsNone() const;
    void ClearCurrentLink();

    void SetUserInputState(const UserInputState& state) { m_userInputState = state; }
    UserInputState GetUserInputState() const { return m_userInputState; }

    SelectionSquare GetSelectionSquare() const { return m_selectionSquare; }

    void SaveToFile(const std::string& path);
    void LoadFromFile(const std::string& path);
    
    void Serialize(CppSer::Serializer& serializer) const;
    void SerializeSelectedNodes(CppSer::Serializer& serializer) const;

    void Deserialize(CppSer::Parser& parser);
    SerializedData DeserializeData(CppSer::Parser& parser);

    void Paste();
    
    void Clean();

    Utils::EventWithID<> EOnDrawEvent;

private:
    void SetHoveredStream(const Weak<Stream>& stream);

private:
    friend class NodeWindow;
    friend class ShaderMaker;

    std::filesystem::path m_savePath;
    
    NodeWindow* m_parent;
    LinkManager* m_linkManager = nullptr;
    NodeList m_nodes;
    
    Link m_currentLink; // The link when creating a new link
    std::vector<NodeWeak> m_selectedNodes;
    Weak<Stream> m_hoveredStream;

    InputWeak m_currentInput;

    UserInputState m_userInputState = UserInputState::None;

    bool m_firstFrame = true;
    
    bool m_isGridHovered = true;
    Vec2f m_onClickPos;
    SelectionSquare m_selectionSquare;
};
