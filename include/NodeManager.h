#pragma once
#include <galaxymath/Maths.h>
#include <memory>
#include <vector>


class Node;
struct Input;
struct Output;

typedef std::shared_ptr<Node> NodeRef;
typedef std::weak_ptr<Node> NodeWeakRef;

typedef std::shared_ptr<Input> InputRef;
typedef std::weak_ptr<Input> InputWeakRef;

typedef std::shared_ptr<Output> OutputRef;
typedef std::weak_ptr<Output> OutputWeakRef;


class NodeManager
{
public:
    void AddNode(NodeRef node);

    void DrawNodes(float zoom, const Vec2f& origin, const Vec2f& mousePos);

    void SelectNode(NodeRef node);
    

private:
    using NodeList = std::vector<NodeRef>;
    NodeList m_nodes;

    NodeWeakRef m_selectedNode;
    InputWeakRef m_selectedInput;

    Vec2f m_firstClickOffset;
    Vec2f m_defaultPosition;

    
};
