#pragma once
#include <galaxymath/Maths.h>
#include <memory>
#include <unordered_map>

#include "LinkManager.h"
#include "UUID.h"

#include "Node.h"


class LinkManager;
using NodeList = std::pmr::unordered_map<UUID, NodeRef>;
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
    void UpdateDelete();
    void OnInputClicked(const NodeRef& node, bool altClicked, size_t i);
    void OnOutputClicked(const NodeRef& node, bool altClicked, size_t i);
    void UpdateInOut(float zoom, const Vec2f& origin, const Vec2f& mousePos, bool mouseClicked, const NodeRef& node);
    void UpdateCurrentLink();

    void DrawNodes(float zoom, const Vec2f& origin, const Vec2f& mousePos) const;
    void UpdateNodes(float zoom, const Vec2f& origin, const Vec2f& mousePos);

    void SelectNode(const NodeRef& node);
    
    LinkManager* GetLinkManager() const { return m_linkManager; }
    NodeWeakRef GetNode(const UUID& uuid) const{ return m_nodes.at(uuid); }
    InputWeakRef GetInput(const UUID& uuid, uint32_t index) { return m_nodes[uuid]->GetInput(index); }
    OutputWeakRef GetOutput(const UUID& uuid, uint32_t index) { return m_nodes[uuid]->GetOutput(index); }
    LinkWeakRef GetLinkWithOutput(const UUID& uuid, uint32_t index) const;

    float GetZoom() const { return m_zoom; }
    Vec2f GetOrigin() const { return m_origin; }

    // Link
    LinkWeakRef GetSelectedLink() const { return m_selectedLink; }
    bool IsAlmostLinked() const;
private:
    static std::unique_ptr<NodeManager> m_instance;
    
    LinkManager* m_linkManager = nullptr;
    
    NodeList m_nodes;

    Link m_currentLink; // The link when creating a new link

    NodeWeakRef m_selectedNode;
    LinkWeakRef m_selectedLink;

    Vec2f m_firstClickOffset;
    Vec2f m_defaultPosition;

    float m_zoom;
    Vec2f m_origin;
};
