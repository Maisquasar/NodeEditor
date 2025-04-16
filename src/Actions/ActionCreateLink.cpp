#include "Actions/ActionCreateLink.h"

ActionCreateLink::ActionCreateLink(NodeManager* nodeManager, const LinkRef& link): m_nodeManager(nodeManager), m_link(link)
{
    
}

void ActionCreateLink::Do()
{
    m_nodeManager->GetLinkManager()->AddLink(m_link);
}

void ActionCreateLink::Undo()
{
    m_nodeManager->GetLinkManager()->RemoveLink(m_link);
}

std::unordered_set<UUID> ActionCreateLink::NodeToUpdate() const
{
    return {m_link->toNodeIndex};
}
