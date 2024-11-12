#include "Actions/ActionCreateNode.h"

#include "NodeManager.h"
#include "NodeTemplateHandler.h"

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
