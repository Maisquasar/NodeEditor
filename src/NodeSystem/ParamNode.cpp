#include "NodeSystem/ParamNode.h"

#include <CppSerializer.h>
#include <imgui_stdlib.h>

#include "NodeEditor.h"
#include "NodeWindow.h"
#include "Type.h"
#include "Actions/Action.h"
#include "Actions/ActionChangeInput.h"
#include "Actions/ActionChangeType.h"

void ParamNodeManager::AddParamNode(ParamNode* node, const std::string& name)
{
    auto it = m_paramNodes.find(name);
    if (it == m_paramNodes.end())
    {
        std::vector<ParamNode*> nodes;
        nodes.push_back(node);
        m_paramNodes[name] = nodes;
    }
    else
    {
        it->second.push_back(node);
    }
}

void ParamNodeManager::RemoveParamNode(ParamNode* node, const std::string& name)
{
    auto it = m_paramNodes.find(name);
    assert(it != m_paramNodes.end());
    std::vector<ParamNode*>& nodes = it->second;
    auto nodeIt = std::ranges::find(nodes, node);
    if (nodeIt != nodes.end())
    {
        nodes.erase(nodeIt);
        if (nodes.empty())
        {
            m_paramNodes.erase(it);
        }
    }
}

bool ParamNodeManager::Exist(const std::string& name) const
{
    return m_paramNodes.contains(name);
}

void ParamNodeManager::UpdateValue(const std::string& name, Vec4f value)
{
    auto list = m_paramNodes.find(name);
    assert(list != m_paramNodes.end());

    for (ParamNode* node : list->second)
    {
        node->SetPreviewValue(value);
    }
}

void ParamNodeManager::UpdateType(const std::string& name, Type type)
{
    auto list = m_paramNodes.find(name);
    assert(list != m_paramNodes.end());

    for (ParamNode* node : list->second)
    {
        node->SetType(type);
    }
}

void ParamNodeManager::OnUpdateName(ParamNode* node, const std::string& prevName, const std::string& newName)
{
    auto prevList = m_paramNodes.find(prevName);
    assert(prevList != m_paramNodes.end());

    // Remove from previous list
    RemoveParamNode(node, prevName);

    // Add to new list
    AddParamNode(node, newName);
}

void ParamNode::ShowInInspector()
{
    Node::ShowInInspector();

    ImGui::BeginDisabled(!m_editable);

    ImGui::SeparatorText("Output");

    if (ImGui::InputText("Param Name", &m_paramName))
    {
        Ref<ActionChangeInput> changeInput = std::make_shared<ActionChangeInput>(&p_outputs.back()->name, p_outputs.back()->name, m_paramName);
        ActionManager::AddAction(changeInput);

        p_nodeManager->GetParamManager()->OnUpdateName(this, p_outputs.back()->name, m_paramName);
        p_outputs.back()->name = m_paramName;
    }

    m_paramName = p_outputs.back()->name;

    int type = static_cast<int>(m_paramType) - 1;
    if (ImGui::Combo("Type", &type, SerializeTypeEnum()))
    {
        Type realType = static_cast<Type>(type + 1);
        Ref<ActionChangeType> changeType = std::make_shared<ActionChangeType>(this, realType, m_paramType);
        SetType(realType);
        p_nodeManager->GetParamManager()->UpdateType(m_paramName, realType);
        ActionManager::AddAction(changeType);
    }

    const char* InputName = "Preview Value";
    Vec4f previewValue = GetPreviewValue();
    bool valueChanged = false;
    //TODO Add action for each type
    switch (m_paramType)
    {
    case Type::None:
        assert(false);
        break;
    case Type::Float:
        {
            if (ImGui::DragFloat(InputName, &previewValue.x, 0.1f))
            {
                valueChanged = true;
            }
        }
        break;
    case Type::Int:
        {
            int val = static_cast<int>(previewValue.x);
            if (ImGui::DragInt(InputName, &val, 1))
            {
                previewValue.x = static_cast<float>(val);
                valueChanged = true;
            }
        }
        break;
    case Type::Bool:
        {
            bool val = previewValue.x > 0.0f;
            if (ImGui::Checkbox(InputName, &val))
            {
                previewValue.x = val ? 1.0f : 0.0f;
                valueChanged = true;
            }
        }
        break;
    case Type::Vector2:
        {
            Vec2f val = Vec2f(previewValue.x, previewValue.y);
            if (ImGui::DragFloat2(InputName, &previewValue.x, 0.1f))
            {
                valueChanged = true;
            }
        }
        break;
    case Type::Vector3:
        {
            if (ImGui::ColorEdit3(InputName, &previewValue.x))
            {
                valueChanged = true;
            }
        }
        break;
    case Type::Vector4:
        {
            if (ImGui::ColorEdit4(InputName, &previewValue.x))
            {
                valueChanged = true;
            }
        }
        break;
    case Type::Sampler2D:
        {
            int valueInt = static_cast<int>(previewValue.x);
            if (NodeEditor::ShowTextureSelector("##texture", &valueInt))
            {
                previewValue.x = static_cast<float>(valueInt);
                valueChanged = true;
            }
        }
        break;
    default:
        break;
    }
    
    ImGui::EndDisabled();

    if (valueChanged)
    {
        SetPreviewValue(previewValue);
        p_nodeManager->GetMainWindow()->ShouldUpdateShader();
        p_nodeManager->GetParamManager()->UpdateValue(m_paramName, previewValue);
    }
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
    if (m_previewValue.has_value())
        serializer << CppSer::Pair::Key << "Preview Value" << CppSer::Pair::Value << m_previewValue.value();
}

void ParamNode::InternalDeserialize(CppSer::Parser& parser)
{
    Node::InternalDeserialize(parser);

    m_paramName = parser["Param Name"].As<std::string>();
    // Do not use the method to not remove links associate to it, force the type set
    m_paramType = static_cast<Type>(parser["Param Type"].As<int>());

    // if (m_paramType == Type::Sampler2D)
    SetType(m_paramType);

    if (parser.HasKey("Preview Value"))
        m_previewValue = parser["Preview Value"].As<Vec4f>();
}

Node* ParamNode::Clone() const
{
    auto node = new ParamNode(p_name);
    node->m_paramName = m_paramName;
    p_outputs.back()->name = m_paramName;
    node->m_paramType = m_paramType;
    node->m_editable = m_editable;
    node->m_serialize = m_serialize;
    node->m_previewValue = m_previewValue;
    Internal_Clone(node);
    return node;
}

void ParamNode::OnCreate()
{
    if (!m_editable)
        return;
    std::string paramName;
    int index = 0;
    do
    {
        if (index == 0)
            paramName = "None";
        else
            paramName = "None" + std::to_string(index);
        index++;
    }
    while (p_nodeManager->GetParamManager()->Exist(paramName));
    SetParamName(paramName);
    p_nodeManager->GetParamManager()->AddParamNode(this, GetParamName());
}

void ParamNode::OnRemove()
{
    if (!m_editable)
        return;
    p_nodeManager->GetParamManager()->RemoveParamNode(this, GetParamName());
}

void ParamNode::SetParamName(std::string name)
{
    m_paramName = std::move(name);
    p_outputs.back()->name = m_paramName;
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
