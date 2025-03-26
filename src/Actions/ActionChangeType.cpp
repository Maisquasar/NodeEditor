#include "Actions/ActionChangeType.h"

#include "NodeSystem/CustomNode.h"
#include "NodeSystem/ParamNode.h"

ActionChangeType::ActionChangeType(ParamNode* node, Type type, Type oldType): Action(), m_paramNode(node), m_type(type), m_oldType(oldType)
{
    auto links = m_paramNode->GetNodeManager()->GetLinkManager()->GetLinksWithOutput(m_paramNode->GetOutput(0));
    for (auto& link : links)
    {
        m_link.push_back(*link.lock());
    }
}

ActionChangeType::ActionChangeType(CustomNode* node, InputRef input, Type type, Type oldType) : Action(), m_customNode(node), m_type(type), m_oldType(oldType), m_input(input)
{
    auto links = m_customNode->GetNodeManager()->GetLinkManager()->GetLinksWithInput(node->GetUUID(), input->index);
    for (auto& link : links)
    {
        m_link.push_back(*link.lock());
    }
}

ActionChangeType::ActionChangeType(CustomNode* node, OutputRef output, Type type, Type oldType) : Action(), m_customNode(node), m_type(type), m_oldType(oldType), m_output(output)
{
    auto links = m_customNode->GetNodeManager()->GetLinkManager()->GetLinksWithOutput(output);
    for (auto& link : links)
    {
        m_link.push_back(*link.lock());
    }
}

void ActionChangeType::Do()
{
    if (m_paramNode)
    {
        m_paramNode->SetType(m_type);
        m_paramNode->GetNodeManager()->GetParamManager()->UpdateType(m_paramNode->GetParamName(), m_type);
    }
    else if (m_customNode)
    {
        if (m_input)
        {
            m_customNode->RemoveInput(m_input->index);
            m_customNode->AddInput(m_input->name, m_type);
        }
        else if (m_output)
        {
            m_customNode->RemoveOutput(m_output->index);
            m_customNode->AddOutput(m_output->name, m_type);
        }
    }
}

void ActionChangeType::Undo()
{
    Node* node;
    if (m_paramNode)
    {
        node = m_paramNode;
        m_paramNode->SetType(m_oldType);
        m_paramNode->GetNodeManager()->GetParamManager()->UpdateType(m_paramNode->GetParamName(), m_oldType);
    }
    else if (m_customNode)
    {
        node = m_customNode;
        if (m_input)
        {
            m_customNode->RemoveInput(m_input->index);
            m_customNode->AddInput(m_input->name, m_oldType);
        }
        else if (m_output)
        {
            m_customNode->RemoveOutput(m_output->index);
            m_customNode->AddOutput(m_output->name, m_oldType);
        }
    }
        
    for (auto& link : m_link)
    {
        node->GetNodeManager()->GetLinkManager()->AddLink(link);
    }
    
}
