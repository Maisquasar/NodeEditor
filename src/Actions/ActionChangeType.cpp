#include "Actions/ActionChangeType.h"

#include "NodeSystem/ParamNode.h"

ActionChangeType::ActionChangeType(ParamNode* node, Type type, Type oldType): Action(), m_node(node), m_type(type), m_oldType(oldType)
{
    auto links = m_node->GetNodeManager()->GetLinkManager()->GetLinksWithOutput(m_node->GetOutput(0));
    for (auto& link : links)
    {
        m_link.push_back(*link.lock());
    }
}

void ActionChangeType::Do()
{
    m_node->SetType(m_type);
}

void ActionChangeType::Undo()
{
    m_node->SetType(m_oldType);
    for (auto& link : m_link)
    {
        m_node->GetNodeManager()->GetLinkManager()->AddLink(link);
    }
}
