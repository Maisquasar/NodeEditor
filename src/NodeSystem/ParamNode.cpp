#include "NodeSystem/ParamNode.h"

#include <CppSerializer.h>
#include <imgui_stdlib.h>

#include "NodeEditor.h"
#include "NodeWindow.h"
#include "Type.h"
#include "Actions/Action.h"
#include "Actions/ActionChangeInput.h"
#include "Actions/ActionChangeType.h"
#include "Actions/ActionChangeValue.h"

void ParamNodeManager::AddParamNode(ParamNode* node, const std::string& name)
{
    if (!m_nodeManager)
    {
        m_nodeManager = node->GetNodeManager();
    }
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

        UpdateType(name, it->second[0]->GetType());
        UpdateValue(name, it->second[0]->GetPreviewValue());
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
        if (node->GetType() == type)
            continue;
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

void ParamNodeManager::OnUpdateType(const std::string& name) const
{
    auto list = GetParamNodes(name);
    LinkManager* linkManager = nullptr;
    for (ParamNode* node : list)
    {
        if (!linkManager)
            linkManager = node->GetNodeManager()->GetLinkManager();
        linkManager->RemoveLinks(node->GetOutput(0));
        linkManager->RemoveLink(node->GetInput(0));
    }
}

void ParamNode::ShowInInspector()
{
    Node::ShowInInspector();

    ImGui::BeginDisabled(!m_editable);

    ImGui::SeparatorText("Output");

    if (ImGui::InputText("Param Name", &m_paramName))
    {
        Ref changeName = std::make_shared<ActionChangeStreamNameParam>(this, &p_outputs.back()->name, p_outputs.back()->name, m_paramName);
        ActionManager::DoAction(changeName);
    }

    m_paramName = p_outputs.back()->name;

    int type = static_cast<int>(m_paramType) - 1;
    if (ImGui::Combo("Type", &type, SerializeTypeEnum()))
    {
        Type realType = static_cast<Type>(type + 1);
        Ref changeType = std::make_shared<ActionChangeTypeParam>(GetNodeManager()->GetParamManager(), m_paramName, realType, m_paramType);
        ActionManager::DoAction(changeType);
    }

    const char* InputName = "Preview Value";
    Vec4f previewValue = GetPreviewValue();
    bool valueChanged = false;
    std::filesystem::path path = "";
    static bool s_firstFrame = true;
    static Vec4f s_valueAtStart = Vec4f(0);
    //TODO Add action for each type
    switch (m_paramType)
    {
    case Type::None:
        assert(false);
        break;
    case Type::Float:
        {
            if (ImGui::InputFloat(InputName, &previewValue.x, 0.1f))
            {
                valueChanged = true;
            }
        }
        break;
    case Type::Int:
        {
            int val = static_cast<int>(previewValue.x);
            if (ImGui::InputInt(InputName, &val, 1))
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
            if (ImGui::DragFloat2(InputName, &previewValue.x))
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
            if (NodeEditor::ShowTextureSelector("##texture", &valueInt, &path))
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
        if (s_firstFrame)
        {
            s_valueAtStart = m_previewValue.value_or(Vec4f());
            s_firstFrame = false;
        }
        p_nodeManager->GetMainWindow()->ShouldUpdateShader();
        p_nodeManager->GetParamManager()->UpdateValue(GetParamName(), previewValue);
        SetTexturePath(path);
    }
    if (!s_firstFrame && (ImGui::IsItemDeactivatedAfterEdit() || m_paramType == Type::Sampler2D && valueChanged)) // Do this to avoid calling each frame
    {
        Ref changeValue = std::make_shared<ActionChangePreviewValue>(
            this, s_valueAtStart, previewValue, m_texturePath.value_or(""), path);
        ActionManager::AddAction(changeValue);
        std::cout << "Change value from " << s_valueAtStart.ToString() << " to " << previewValue.ToString() << std::endl;
        s_valueAtStart = Vec4f();
        s_firstFrame = true;
    }
}

std::vector<std::string> ParamNode::GetFormatStrings() const
{
    if (m_paramType == Type::Sampler2D)
    {
        if (DoesInputHaveLink(0))
            return {"texture2D(" + m_paramName + ", %s)"};    
        return {"texture2D(" + m_paramName + ", " + NodeEditor::TexCoordsVariableName + ")"};
    }
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

    std::string prevName = parser["Param Name"].As<std::string>();
    if (m_editable)
        p_nodeManager->GetParamManager()->OnUpdateName(this, m_paramName, prevName);
    m_paramName = prevName;
    // Do not use the method to not remove links associate to it, force the type set
    Type newType = static_cast<Type>(parser["Param Type"].As<int>());

    SetType(newType);

    if (m_paramType != Type::Sampler2D && parser.HasKey("Preview Value"))
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
    ClearInputs();
    
    m_paramType = type;
    if (type == Type::Sampler2D)
    {
        type = Type::Vector4;
        AddInput("UV", Type::Vector2);
    }
    AddOutput(m_paramName, type);
}

std::string ParamNode::ToShader(ShaderMaker* shaderMaker, const FuncStruct& funcStruct) const
{
    return Node::ToShader(shaderMaker, funcStruct);
}
