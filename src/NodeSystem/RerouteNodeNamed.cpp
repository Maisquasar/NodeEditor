#include "NodeSystem/RerouteNodeNamed.h"

#include <CppSerializer.h>
#include <imgui_stdlib.h>

#include "NodeWindow.h"
#include "Actions/Action.h"
#include "NodeSystem/NodeManager.h"
#include "NodeSystem/NodeTemplateHandler.h"
#include "NodeSystem/ShaderMaker.h"

class ActionChangeType;

RerouteNodeNamedManager::~RerouteNodeNamedManager()
{
    Clean();
}

void RerouteNodeNamedManager::Clean()
{
    auto instance = NodeTemplateHandler::GetInstance();
    for (auto& [name, nodeData] : m_rerouteNamedNodes)
    {
        instance->RemoveTemplateNode(name);
    }
    m_rerouteNamedNodes.clear();
}

RerouteNodeNamedData* RerouteNodeNamedManager::GetNode(const std::string& name)
{
    auto it = m_rerouteNamedNodes.find(name);
    if (it == m_rerouteNamedNodes.end())
    {
        std::cerr << "Node with name " << name << " does not exist\n";
        return nullptr;
    }
    return &m_rerouteNamedNodes[name];
}

void RerouteNodeNamedManager::UpdateKey(std::string oldName, const std::string& newName)
{
    auto it = m_rerouteNamedNodes.find(oldName);
    if (it == m_rerouteNamedNodes.end())
    {
        std::cerr << "Node with old name " << oldName << " does not exist\n";
        return;
    }
    if (m_rerouteNamedNodes.contains(newName))
    {
        std::cerr << "Node with new name " << newName << " already exists\n";
        return;
    }

    auto nodeData = std::move(it->second);
    nodeData.name = newName;
    m_rerouteNamedNodes.erase(it);
    m_rerouteNamedNodes[newName] = nodeData;
    
    for (RerouteNodeNamed*& node : nodeData.node)
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

void RerouteNodeNamedManager::UpdateColor(const std::string& name, const Vec3f& color)
{
    auto it = GetNode(name);
    if (it == nullptr)
        return;
    for (auto& node : it->node)
    {
        node->SetColor(color);
    }
    NodeTemplateHandler::UpdateColor(name, IM_COL32(color.x * 255.f, color.y * 255.f, color.z * 255.f, 255.f));
}

void RerouteNodeNamedManager::AddNode(const std::string& name)
{
    if (m_rerouteNamedNodes.contains(name))
    {
        std::cerr << "Node with name " << name << " already exists\n";
        return;
    }
    m_rerouteNamedNodes[name] = {.name= name};
}

void RerouteNodeNamedManager::RemoveNode(const std::string& name)
{
    if (RerouteNodeNamedData* node = GetNode(name))
    {
        m_rerouteNamedNodes.erase(name);
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
    return m_rerouteNamedNodes.contains(name);
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
        if (NodeTemplateHandler::DoesNameExist(m_name))
        {
            std::cerr << "Node with name " << m_name << " already exists\n";
            SetRerouteName(oldName);
        }
        else
        {
            p_nodeManager->GetRerouteManager()->UpdateKey(oldName, m_name);
        }
    }
    
    int type = static_cast<int>(m_type) - 1;
    if (ImGui::Combo("Type", &type, SerializeTypeEnum()))
    {
        //TODO : Add action
        // Ref<ActionChangeType> changeType = std::make_shared<ActionChangeType>(this, static_cast<Type>(type + 1), m_paramType);
        SetType(static_cast<Type>(type + 1));
        p_nodeManager->GetRerouteManager()->UpdateType(p_name, m_type);
        // ActionManager::AddAction(changeType);
    }

    Vec3f color = m_color.value();
    if (ImGui::ColorEdit3("Node Color", &color.x))
    {
        SetColor(color);
        p_nodeManager->GetRerouteManager()->UpdateColor(p_name, color);
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
        
        std::string thisContent = "\t" + glslType + " " + variableName + " = ";
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
        
        std::string parentVariableName;
        if (funcStruct.inputs.empty()) // No Definition !
            variableNames.push_back(ShaderMaker::ToGLSLVariable(p_outputs[0]->type, Vec4f(0)));
        else
            parentVariableName = funcStruct.inputs[0];
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
    ChangeOutputType(0, type);
    if (!m_definition && !p_inputs.empty())
    {
        RemoveInput(0);
    }
    else if (m_definition && p_inputs.empty())
    {
        AddInput("", type);
    }
    else if (!p_inputs.empty())
    {
        ChangeInputType(0, type);
    }
    m_type = type;
}

void RerouteNodeNamed::SetColor(const Vec3f& color)
{
    m_color = color;
    SetTopColor(IM_COL32(color.x * 255.f, color.y * 255.f, color.z * 255.f, 255.f));
}

void RerouteNodeNamed::InternalSerialize(CppSer::Serializer& serializer) const
{
    Node::InternalSerialize(serializer);

    serializer << CppSer::Pair::Key << "Name" << CppSer::Pair::Value << m_name;
    serializer << CppSer::Pair::Key << "Type" << CppSer::Pair::Value << static_cast<int>(m_type);
    serializer << CppSer::Pair::Key << "Definition" << CppSer::Pair::Value << m_definition;
    serializer << CppSer::Pair::Key << "Color" << CppSer::Pair::Value << m_color.value();
}

void RerouteNodeNamed::InternalDeserialize(CppSer::Parser& parser)
{
    Node::InternalDeserialize(parser);

    auto type = parser["Type"].As<int>();
    auto name = parser["Name"].As<std::string>();
    auto definition = parser["Definition"].As<bool>();
    auto def = p_nodeManager->GetRerouteManager()->GetDefinitionNode(name);
    m_definition = definition && (def == nullptr || def == this);
    SetType(static_cast<Type>(type));
    p_nodeManager->GetRerouteManager()->UpdateKey(p_name, name);
    p_nodeManager->GetRerouteManager()->UpdateType(p_name, m_type);
    SetRerouteName(name);
    SetName(m_name);
    m_color = parser["Color"].As<Vec3f>();
    p_nodeManager->GetRerouteManager()->UpdateColor(p_name, m_color.value());
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

void RerouteNodeNamed::InitializePreview()
{
    if (!m_definition)
    {
        Ref<RerouteNodeNamed> nodeDefinition = GetDefinitionNode();
        m_shader = nodeDefinition->m_shader;
        if (!m_shader)
        {
            nodeDefinition->InitializePreview();
            m_shader = nodeDefinition->m_shader;
        }
        m_framebuffer = nodeDefinition->m_framebuffer;
        p_nodeManager->GetMainWindow()->ShouldUpdateShader();
    }
    else
    {
        Node::InitializePreview();
    }
}

Ref<RerouteNodeNamed> RerouteNodeNamed::GetDefinitionNode() const
{
    RerouteNodeNamed* rerouteNodeNamed = p_nodeManager->GetRerouteManager()->GetDefinitionNode(m_name);
    if (!rerouteNodeNamed)
        return nullptr;
    auto lock = p_nodeManager->GetNode(rerouteNodeNamed->p_uuid).lock();
    return std::dynamic_pointer_cast<RerouteNodeNamed>(lock);
}

void RerouteNodeNamed::SetRerouteName(const std::string& string)
{
    m_name = string;
    SetName(string);
    RecalculateWidth();
}

void RerouteNodeNamed::OnCreate()
{
    if (m_templateNode || !p_nodeManager->GetRerouteManager()->HasDefinition(m_name))
    {
        NodeRef templateNode = NodeTemplateHandler::GetNodeFromName(m_name + " (Definition)");
        
        m_templateNode = false;
        int index = 0;
        SetName(m_name);

        std::string defaultName = m_name;
        while (NodeTemplateHandler::DoesNameExist(m_name))
        {
            auto newName = defaultName + "_" + std::to_string(index);
            
            m_name = newName;
            SetName(newName);
        }

        if (!templateNode)
        {
            auto templateHandler = NodeTemplateHandler::GetInstance();
            RerouteNodeNamed* clone = dynamic_cast<RerouteNodeNamed*>(Clone());
            clone->p_nodeManager = p_nodeManager;
            const NodeMethodInfo info(clone);
            templateHandler->AddTemplateNode(info);
        }
        else
        {
            templateNode->SetName(m_name);
        }

        p_nodeManager->GetRerouteManager()->AddNode(m_name);

        m_definition = true;
        if (p_inputs.empty())
            AddInput("", m_type);
        else
            ChangeInputType(0, m_type);
    }
    p_nodeManager->GetRerouteManager()->AddRef(m_name, this);
    
    if (!m_color.has_value())
    {
        p_nodeManager->GetRerouteManager()->UpdateColor(m_name, Vec4f(ImGui::ColorConvertU32ToFloat4(p_topColor)));
    }
}

void RerouteNodeNamed::OnRemove()
{
    Node::OnRemove();
    
    p_nodeManager->GetRerouteManager()->RemoveRef(m_name, this);

    if (m_definition && p_nodeManager->GetRerouteManager()->HasNode(m_name))
    {
        NodeRef templateNode = NodeTemplateHandler::GetNodeFromName(m_name);
        templateNode->SetName(m_name + " (Definition)");
    }
}
