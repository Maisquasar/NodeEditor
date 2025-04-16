#include "Actions/ActionChangeValue.h"

#include "NodeWindow.h"

void ActionChangeValue::Do()
{
    *value = newValue;
}

void ActionChangeValue::Undo()
{
    *value = oldValue;
}

std::string ActionChangeValue::ToString()
{
    return "Change Value";
}

std::unordered_set<UUID> ActionChangeValue::NodeToUpdate() const
{
    return {nodeUUID};
}

ActionChangePreviewValue::ActionChangePreviewValue(ParamNode* paramNode, const Vec4f& oldValue, const Vec4f& newValue,
                                                   const std::filesystem::path& oldTexturePath, const std::filesystem::path& newTexturePath)
{
    m_paramNode = paramNode;
    m_oldValue = oldValue;
    m_newValue = newValue;
    m_oldTexturePath = oldTexturePath;
    m_newTexturePath = newTexturePath;
}

void ActionChangePreviewValue::Do()
{
    m_paramNode->GetNodeManager()->GetParamManager()->UpdateValue(m_paramNode->GetParamName(), m_newValue);
    m_paramNode->SetTexturePath(m_newTexturePath);
}

void ActionChangePreviewValue::Undo()
{
    m_paramNode->GetNodeManager()->GetParamManager()->UpdateValue(m_paramNode->GetParamName(), m_oldValue);
    m_paramNode->SetTexturePath(m_oldTexturePath);
}

std::unordered_set<UUID> ActionChangePreviewValue::NodeToUpdate() const
{
    return {m_paramNode->GetUUID()};
}
