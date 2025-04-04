#include "Actions/ActionChangeInput.h"

#include "NodeSystem/ParamNode.h"

ActionChangeStreamNameParam::ActionChangeStreamNameParam(ParamNode* node, std::string* input, const std::string& oldValue,
                                               const std::string& newValue)
{
    m_node = node;
    m_input = input;
    m_oldValue = oldValue;
    m_newValue = newValue;
}

void ActionChangeStreamNameParam::Do()
{
    m_node->GetNodeManager()->GetParamManager()->OnUpdateName(m_node, m_oldValue, m_newValue);
    m_node->SetParamName(m_newValue);
}

void ActionChangeStreamNameParam::Undo()
{
    m_node->GetNodeManager()->GetParamManager()->OnUpdateName(m_node, m_newValue, m_oldValue);
    m_node->SetParamName(m_oldValue);
}

ActionChangeStreamNameCustom::ActionChangeStreamNameCustom(CustomNode* node, std::string* input, std::string oldValue, std::string newValue):
    m_input(input), m_oldValue(std::move(oldValue)), m_newValue(std::move(newValue))
{
    m_node = node;
}

void ActionChangeStreamNameCustom::Do()
{
    *m_input = m_newValue;
    m_node->RecalculateWidth();
    m_node->UpdateFunction();
}

void ActionChangeStreamNameCustom::Undo()
{
    *m_input = m_oldValue;
    m_node->RecalculateWidth();
    m_node->UpdateFunction();
}
