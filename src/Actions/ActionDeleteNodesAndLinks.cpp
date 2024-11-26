#include "Actions/ActionDeleteNodesAndLinks.h"

ActionDeleteNodesAndLinks::ActionDeleteNodesAndLinks(NodeManager* nodeManager, const std::vector<NodeWeak>& nodes, const std::vector<LinkWeakRef>& links)
: m_nodeManager(nodeManager)
{
    for (auto& node : nodes)
    {
        m_nodes.push_back(node.lock());
    }
    for (auto& link : links)
    {
        m_links.push_back(link.lock());
    }
}

void ActionDeleteNodesAndLinks::Do()
{
    for (LinkRef& link : m_links)
    {
        m_nodeManager->GetLinkManager()->RemoveLink(link);
    }
    for (NodeRef& node : m_nodes)
    {
        m_nodeManager->RemoveNode(node);
    }
}

void ActionDeleteNodesAndLinks::Undo()
{
    for (LinkRef& link : m_links)
    {
        m_nodeManager->GetLinkManager()->AddLink(link);
    }
    for (NodeRef& node : m_nodes)
    {
        m_nodeManager->AddNode(node);
    }
}