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

    void DrawNodes(float zoom, const Vec2f& origin, const Vec2f& mousePos);

    void SelectNode(NodeRef node);
    
    NodeWeakRef GetNode(const UUID& uuid) { return m_nodes[uuid]; }
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
