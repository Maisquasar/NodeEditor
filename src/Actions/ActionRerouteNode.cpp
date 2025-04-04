#include "Actions/ActionRerouteNode.h"

#include "NodeSystem/NodeManager.h"
#include "NodeSystem/NodeTemplateHandler.h"
#include "NodeSystem/RerouteNodeNamed.h"

void ActionRenameRerouteNode::Do()
{
    if (NodeTemplateHandler::DoesNameExist(m_newName))
    {
        std::cerr << "Node with name " << m_newName << " already exists\n";
        m_node->SetRerouteName(m_oldName);
    }
    else
    {
        m_node->GetNodeManager()->GetRerouteManager()->UpdateKey(m_oldName, m_newName);
    }
}

void ActionRenameRerouteNode::Undo()
{
    m_node->GetNodeManager()->GetRerouteManager()->UpdateKey(m_newName, m_oldName);
}

ActionChangeColorRerouteNode::ActionChangeColorRerouteNode(RerouteNodeNamedManager* manager, const std::string& name,
    Vec4f prevColor, Vec4f newColor)
{
    m_rerouteNodeNamedManager = manager;
    m_name = name;
    m_prevColor = prevColor;
    m_newColor = newColor;
}

void ActionChangeColorRerouteNode::Do()
{
    m_rerouteNodeNamedManager->UpdateColor(m_name, m_newColor);
}

void ActionChangeColorRerouteNode::Undo()
{
    m_rerouteNodeNamedManager->UpdateColor(m_name, m_prevColor);
}
