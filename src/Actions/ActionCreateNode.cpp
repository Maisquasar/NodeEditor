#include "Actions/ActionCreateNode.h"

#include "NodeSystem/NodeManager.h"

ActionCreateNode::ActionCreateNode(NodeManager* nodeManager, NodeRef node) : m_nodeManager(nodeManager), m_node(node)
{
}

void ActionCreateNode::Do()
{
    m_nodeManager->AddNode(m_node);
}

void ActionCreateNode::Undo()
{
    m_nodeManager->RemoveNode(m_node);
}

std::unordered_set<UUID> ActionCreateNode::NodeToUpdate() const
{
    return {m_node->GetUUID()};
}
