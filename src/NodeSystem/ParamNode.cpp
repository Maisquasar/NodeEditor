#include "NodeSystem/ParamNode.h"

#include <CppSerializer.h>
#include <imgui_stdlib.h>

#include "NodeEditor.h"
#include "NodeWindow.h"
#include "Type.h"
#include "Actions/Action.h"
#include "Actions/ActionChangeInput.h"
#include "Actions/ActionChangeType.h"

void ParamNode::ShowInInspector()
{
    Node::ShowInInspector();

    ImGui::BeginDisabled(!m_editable);

    ImGui::SeparatorText("Output");

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

    const char* InputName = "Preview Value";
    //TODO Add action for each type
    switch (m_paramType)
    {
    case Type::None:
        assert(false);
        break;
    case Type::Float:
        {
            float val = m_previewValue.x;
            if (ImGui::DragFloat(InputName, &val, 0.1f))
            {
                m_previewValue.x = val;
                p_nodeManager->GetMainWindow()->ShouldUpdateShader();
            }
        }
        break;
    case Type::Int:
        {
            int val = static_cast<int>(m_previewValue.x);
            if (ImGui::DragInt(InputName, &val, 1))
            {
                m_previewValue.x = static_cast<float>(val);
                p_nodeManager->GetMainWindow()->ShouldUpdateShader();
            }
        }
        break;
    case Type::Bool:
        {
            bool val = m_previewValue.x > 0.0f;
            if (ImGui::Checkbox(InputName, &val))
            {
                m_previewValue.x = val ? 1.0f : 0.0f;
                p_nodeManager->GetMainWindow()->ShouldUpdateShader();
            }
        }
        break;
    case Type::Vector2:
        {
            Vec2f val = Vec2f(m_previewValue.x, m_previewValue.y);
            if (ImGui::DragFloat2(InputName, &val.x, 0.1f))
            {
                m_previewValue.x = val.x;
                p_nodeManager->GetMainWindow()->ShouldUpdateShader();
            }
        }
        break;
    case Type::Vector3:
        {
            Vec3f val = Vec3f(m_previewValue.x, m_previewValue.y, m_previewValue.z);
            if (ImGui::ColorEdit3(InputName, &val.x, 0.1f))
            {
                m_previewValue.x = val.x;
                p_nodeManager->GetMainWindow()->ShouldUpdateShader();
            }
        }
        break;
    case Type::Vector4:
        {
            if (ImGui::ColorEdit4(InputName, &m_previewValue.x, 0.1f))
            {
                p_nodeManager->GetMainWindow()->ShouldUpdateShader();
            }
        }
        break;
    case Type::Sampler2D:
        {
            //TODO
        }
        break;
    default: ;
    }
    
    ImGui::EndDisabled();
}

std::vector<std::string> ParamNode::GetFormatStrings() const
{
    if (m_paramType == Type::Sampler2D)
        return {"texture2D(" + m_paramName + ", " + NodeEditor::TexCoordsVariableName + ")"};
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
    // Do not use the method to not remove links associate to it, force the type set
    m_paramType = static_cast<Type>(parser["Param Type"].As<int>());
}

Node* ParamNode::Clone() const
{
    auto node = new ParamNode(p_name);
    node->m_paramName = m_paramName;
    p_outputs.back()->name = m_paramName;
    node->m_paramType = m_paramType;
    node->m_editable = m_editable;
    node->m_serialize = m_serialize;
    Internal_Clone(node);
    return node;
}

void ParamNode::SetType(Type type)
{
    ClearOutputs();
    m_paramType = type;
    if (type == Type::Sampler2D)
        type = Type::Vector4;
    AddOutput(m_paramName, type);
}

std::string ParamNode::ToShader(ShaderMaker* shaderMaker, const FuncStruct& funcStruct) const
{
    return Node::ToShader(shaderMaker, funcStruct);
}
