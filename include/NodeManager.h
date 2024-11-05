#pragma once
#include <galaxymath/Maths.h>
#include <memory>
#include <unordered_map>
#include "UUID.h"

#include "Node.h"


class NodeManager
{
public:
    void AddNode(NodeRef node);
    void UpdateToLink(float zoom, const Vec2f& origin, const Vec2f& mousePos, bool mouseClicked,
                      const OutputRef& selectedOutput,
                      const NodeRef& node) const;
    void UpdateFromLink(float zoom, const Vec2f& origin, const Vec2f& mousePos, bool mouseClicked,
                        bool& wasInputClicked,
                        NodeRef& node, bool& alreadyOneSelected);

    void DrawNodes(float zoom, const Vec2f& origin, const Vec2f& mousePos) const;
    void UpdateNodes(float zoom, const Vec2f& origin, const Vec2f& mousePos);

    void SelectNode(const NodeRef& node);
    
    NodeWeakRef GetNode(const UUID& uuid) const{ return m_nodes.at(uuid); }
    InputWeakRef GetInput(const UUID& uuid, uint32_t index) { return m_nodes[uuid]->GetInput(index); }
    OutputWeakRef GetOutput(const UUID& uuid, uint32_t index) { return m_nodes[uuid]->GetOutput(index); }

    float GetZoom() const { return m_zoom; }
    Vec2f GetOrigin() const { return m_origin; }
private:
    using NodeList = std::pmr::unordered_map<UUID, NodeRef>;
    NodeList m_nodes;

    NodeWeakRef m_selectedNode;
    OutputWeakRef m_selectedOutput;

    Vec2f m_firstClickOffset;
    Vec2f m_defaultPosition;

    float m_zoom = 0.f;
    Vec2f m_origin;
};
