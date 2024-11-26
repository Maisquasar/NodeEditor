#include "Actions/ActionMoveNodes.h"

#include "NodeSystem/Node.h"

ActionMoveNodes::ActionMoveNodes(const std::vector<NodeWeakRef>& nodes)
{
    for (auto& node : nodes)
    {
        MoveNodeData data;
        data.oldPosition = node.lock()->GetPosition();
        m_positions[node] = data;
    }
}

void ActionMoveNodes::Do()
{
    for (auto& pair : m_positions)
    {
        NodeRef node = pair.first.lock();
        MoveNodeData data = pair.second;
        node->SetPosition(data.position);
    }
}

void ActionMoveNodes::Undo()
{
    for (auto& pair : m_positions)
    {
        NodeRef node = pair.first.lock();
        MoveNodeData data = pair.second;
        node->SetPosition(data.oldPosition);
    }
}

void ActionMoveNodes::Update()
{
    for (auto& pair : m_positions)
    {
        NodeRef node = pair.first.lock();
        MoveNodeData& data = pair.second;
        data.position = node->GetPosition();
    }
}
