#include "NodeSystem/ParamNode.h"

#include <CppSerializer.h>
#include <imgui_stdlib.h>

#include "Actions/Action.h"
#include "Actions/ActionChangeInput.h"
#include "Actions/ActionChangeType.h"

void ParamNode::ShowInInspector()
{
    Node::ShowInInspector();

    ImGui::SeparatorText("Ouput");

    if (ImGui::InputText("Param Name", &m_paramName))
    {
        Ref<ActionChangeInput> changeInput = std::make_shared<ActionChangeInput>(&p_outputs.back()->name, p_outputs.back()->name, m_paramName);
        ActionManager::AddAction(changeInput);
        
        p_outputs.back()->name = m_paramName;
    }

    m_paramName = p_outputs.back()->name;

    int type = static_cast<int>(m_paramType) - 1;
    if (ImGui::Combo("Type", &type, SerializeTypeEnum()))
    {
        Ref<ActionChangeType> changeType = std::make_shared<ActionChangeType>(this, static_cast<Type>(type + 1), m_paramType);
        SetType(static_cast<Type>(type + 1));
        ActionManager::AddAction(changeType);
    }
}

std::vector<std::string> ParamNode::GetFormatStrings() const
{
    return {m_paramName};
}

void ParamNode::InternalSerialize(CppSer::Serializer& serializer) const
{
    Node::InternalSerialize(serializer);

    serializer << CppSer::Pair::Key << "Param Name" << CppSer::Pair::Value << m_paramName;
    serializer << CppSer::Pair::Key << "Param Type" << CppSer::Pair::Value << static_cast<int>(m_paramType);
}

void ParamNode::InternalDeserialize(CppSer::Parser& parser)
{
    Node::InternalDeserialize(parser);

    m_paramName = parser["Param Name"].As<std::string>();
    SetType(static_cast<Type>(parser["Param Type"].As<int>()));
}

Node* ParamNode::Clone() const
{
    auto node = new ParamNode(p_name);
    node->m_paramName = m_paramName;
    node->m_paramType = m_paramType;
    Internal_Clone(node);
    return node;
}

void ParamNode::SetType(Type type)
{
    RemoveOutput(0);
    AddOutput(m_paramName, type);
    m_paramType = type;
}
