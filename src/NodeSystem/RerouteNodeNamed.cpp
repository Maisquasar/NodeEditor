#include "NodeSystem/RerouteNodeNamed.h"

#include <CppSerializer.h>
#include <imgui_stdlib.h>

#include "Actions/Action.h"
#include "NodeSystem/NodeManager.h"
#include "NodeSystem/NodeTemplateHandler.h"
#include "NodeSystem/ShaderMaker.h"

class ActionChangeType;

std::unique_ptr<RerouteNodeNamedManager> RerouteNodeNamedManager::s_instance = std::make_unique<RerouteNodeNamedManager>();

RerouteNodeNamedManager* RerouteNodeNamedManager::GetInstance()
{
    return s_instance.get();
}

RerouteNodeNamedData* RerouteNodeNamedManager::GetNode(const std::string& name)
{
    auto it = s_instance->m_rerouteNamedNodes.find(name);
    if (it == s_instance->m_rerouteNamedNodes.end())
    {
        std::cerr << "Node with name " << name << " does not exist\n";
        return nullptr;
    }
    return &s_instance->m_rerouteNamedNodes[name];
}

void RerouteNodeNamedManager::UpdateKey(std::string oldName, const std::string& newName)
{
    auto it = s_instance->m_rerouteNamedNodes.find(oldName);
    if (it == s_instance->m_rerouteNamedNodes.end())
    {
        std::cerr << "Node with old name " << oldName << " does not exist\n";
        return;
    }
    if (s_instance->m_rerouteNamedNodes.contains(newName))
    {
        std::cerr << "Node with new name " << newName << " already exists\n";
        return;
    }

    auto nodeData = std::move(it->second);
    nodeData.name = newName;
    s_instance->m_rerouteNamedNodes.erase(it);
    s_instance->m_rerouteNamedNodes[newName] = nodeData;
    
    for (auto& node : nodeData.node)
    {
        node->SetRerouteName(newName);
    }
    NodeTemplateHandler::UpdateKey(oldName, newName);
}

void RerouteNodeNamedManager::UpdateType(const std::string& name, Type type)
{
    auto it = GetNode(name);
    if (it == nullptr)
        return;
    for (auto& node : it->node)
    {
        node->SetType(type);
    }
    NodeTemplateHandler::UpdateType(name, type);
}

void RerouteNodeNamedManager::AddNode(const std::string& name)
{
    if (s_instance->m_rerouteNamedNodes.contains(name))
    {
        std::cerr << "Node with name " << name << " already exists\n";
        return;
    }
    s_instance->m_rerouteNamedNodes[name] = {.name= name};
}

void RerouteNodeNamedManager::RemoveNode(const std::string& name)
{
    if (RerouteNodeNamedData* node = GetNode(name))
    {
        s_instance->m_rerouteNamedNodes.erase(name);
        NodeTemplateHandler::GetInstance()->RemoveTemplateNode(name);
    }
}

void RerouteNodeNamedManager::AddRef(const std::string& name, RerouteNodeNamed* node)
{
    auto it = GetNode(name);

    it->node.push_back(node);
    it->refCount++;
}

void RerouteNodeNamedManager::RemoveRef(const std::string& name, RerouteNodeNamed* node)
{
    const auto it = GetNode(name);

    if (it == nullptr)
        return; // Case where the template node is deleting itself

    it->refCount--;
    it->node.erase(std::ranges::remove(it->node, node).begin(), it->node.end());

    if (it->refCount == 0)
    {
        RemoveNode(name);
    }
}

RerouteNodeNamed* RerouteNodeNamedManager::GetDefinitionNode(const std::string& name)
{
    auto it = GetNode(name);
    if (it == nullptr)
        return nullptr;
    for (auto& node : it->node)
    {
        if (node->IsDefinition())
            return node;
    }
    return nullptr;
}

bool RerouteNodeNamedManager::HasDefinition(const std::string& name)
{
    auto it = GetNode(name);
    if (!it)
        return false;
    for (auto& node : it->node)
    {
        if (node->IsDefinition())
            return true;
    }
    return false;
}

bool RerouteNodeNamedManager::HasNode(const std::string& name)
{
    return s_instance->m_rerouteNamedNodes.contains(name);
}

RerouteNodeNamed::RerouteNodeNamed(const std::string& name): Node(name)
{
}

RerouteNodeNamed::~RerouteNodeNamed()
{
}

std::vector<std::string> RerouteNodeNamed::GetFormatStrings() const
{
    return {"%s"};
}

void RerouteNodeNamed::ShowInInspector()
{
    Node::ShowInInspector();

    auto oldName = m_name;
    if (ImGui::InputText("Name", &m_name))
    {
        //TODO : Add action
        // Ref<ActionChangeInput> changeInput = std::make_shared<ActionChangeInput>(&p_outputs.back()->name, p_outputs.back()->name, m_paramName);
        // ActionManager::AddAction(changeInput);
        if (NodeTemplateHandler::IsNameExist(m_name))
        {
            std::cerr << "Node with name " << m_name << " already exists\n";
            SetRerouteName(oldName);
        }
        else
        {
            RerouteNodeNamedManager::UpdateKey(oldName, m_name);
        }
    }
    
    int type = static_cast<int>(m_type) - 1;
    if (ImGui::Combo("Type", &type, SerializeTypeEnum()))
    {
        //TODO : Add action
        // Ref<ActionChangeType> changeType = std::make_shared<ActionChangeType>(this, static_cast<Type>(type + 1), m_paramType);
        SetType(static_cast<Type>(type + 1));
        RerouteNodeNamedManager::UpdateType(p_name, m_type);
        // ActionManager::AddAction(changeType);
    }
}

std::string RerouteNodeNamed::ToShader(ShaderMaker* shaderMaker, const FuncStruct& funcStruct) const
{
    if (IsDefinition())
        return Node::ToShader(shaderMaker, funcStruct);
    std::string content;
    auto& variableNames = shaderMaker->m_variablesNames;
    auto& allVariableNames = shaderMaker->m_allVariableNames;
    auto toFormatList = GetFormatStrings();
    for (int k = 0; k < p_outputs.size(); k++)
    {
        if (funcStruct.outputs.size() <= k)
            break;
        std::string variableName = funcStruct.outputs[k];
        std::string glslType = ShaderMaker::TypeToGLSLType(p_outputs[k]->type);
        
        std::string thisContent = glslType + " " + variableName + " = ";
        if (allVariableNames.contains(variableName))
            continue;
        allVariableNames.insert(variableName);
        
        std::string toFormat = toFormatList[k];
        std::string secondHalf = toFormat;
        toFormat.clear();
        auto defNode = GetDefinitionNode();
        
        size_t index = secondHalf.find_first_of("%") + 2;
        if (index == std::string::npos)
            break;
        std::string firstHalf = secondHalf.substr(0, index);
        if (index != std::string::npos)
            secondHalf = secondHalf.substr(index);
        auto parentVariableName = funcStruct.inputs[0];
        if (parentVariableName.empty())
            parentVariableName = variableNames.back();
        toFormat += FormatString(firstHalf, parentVariableName.c_str());
        
        thisContent += toFormat + secondHalf + ";\n";

        content += thisContent;
    }
    return content;
}

void RerouteNodeNamed::SetType(Type type)
{
    ClearInputs();
    ClearOutputs();
    AddOutput("", type);
    if (m_definition)
        AddInput("", type);
    m_type = type;
}

void RerouteNodeNamed::InternalSerialize(CppSer::Serializer& serializer) const
{
    Node::InternalSerialize(serializer);

    serializer << CppSer::Pair::Key << "Name" << CppSer::Pair::Value << m_name;
    serializer << CppSer::Pair::Key << "Type" << CppSer::Pair::Value << static_cast<int>(m_type);
}

void RerouteNodeNamed::InternalDeserialize(CppSer::Parser& parser)
{
    Node::InternalDeserialize(parser);

    auto type = parser["Type"].As<int>();
    SetType(static_cast<Type>(type));
    auto name = parser["Name"].As<std::string>();
    RerouteNodeNamedManager::UpdateKey(p_name, name);
    RerouteNodeNamedManager::UpdateType(p_name, m_type);
    SetRerouteName(name);
    SetName(m_name);
}

Node* RerouteNodeNamed::Clone() const
{
    auto node = new RerouteNodeNamed(p_name);
    node->m_templateNode = m_templateNode;
    node->m_definition = m_definition;
    node->SetRerouteName(m_name);
    Internal_Clone(node);
    node->SetType(m_type);
    node->p_name = m_name;
    return node;
}

Ref<RerouteNodeNamed> RerouteNodeNamed::GetDefinitionNode() const
{
    RerouteNodeNamed* rerouteNodeNamed = RerouteNodeNamedManager::GetDefinitionNode(m_name);
    auto lock = p_nodeManager->GetNode(rerouteNodeNamed->p_uuid).lock();
    return std::dynamic_pointer_cast<RerouteNodeNamed>(lock);
}

void RerouteNodeNamed::SetRerouteName(const std::string& string)
{
    m_name = string;
    SetName(string);
}

void RerouteNodeNamed::OnCreate()
{
    if (m_templateNode || !RerouteNodeNamedManager::HasDefinition(m_name))
    {
        auto templateNode = NodeTemplateHandler::GetFromName(m_name + " (Definition)");
        
        m_templateNode = false;
        int index = 0;
        SetName(m_name);

        std::string defaultName = m_name;
        while (NodeTemplateHandler::IsNameExist(m_name))
        {
            SetRerouteName(defaultName + "_" + std::to_string(index));
        }

        if (!templateNode.node)
        {
            auto templateHandler = NodeTemplateHandler::GetInstance();
            Node* clone = Clone();
            const NodeMethodInfo info(clone);
            templateHandler->AddTemplateNode(info);
        }
        else
        {
            templateNode.node->SetName(m_name);
        }

        RerouteNodeNamedManager::AddNode(m_name);

        m_definition = true; 
        AddInput("", m_type);
    }
    RerouteNodeNamedManager::AddRef(m_name, this);
}

void RerouteNodeNamed::OnRemove()
{
    Node::OnRemove();
    
    RerouteNodeNamedManager::RemoveRef(m_name, this);

    if (m_definition && RerouteNodeNamedManager::HasNode(m_name))
    {
        auto templateNode = NodeTemplateHandler::GetFromName(m_name);
        templateNode.node->SetName(m_name + " (Definition)");
    }
}
