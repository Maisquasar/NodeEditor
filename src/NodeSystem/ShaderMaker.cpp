#include "NodeSystem/ShaderMaker.h"

#include <fstream>
#include <ranges>

#include "NodeWindow.h"
#include "NodeSystem/CustomNode.h"
#include "NodeSystem/NodeTemplateHandler.h"
#include "NodeSystem/ParamNode.h"
#include "NodeSystem/RerouteNodeNamed.h"
#include "Render/Framebuffer.h"

class RerouteNodeNamed;

std::string ShaderMaker::s_shaderHeader = "#version 330 core\nin vec2 TexCoords;\nuniform float Time;\nout vec4 FragColor;\n\n";;
std::string ShaderMaker::s_shaderMainHeader = "void main()\n{\n";
std::string ShaderMaker::s_shaderFooter = "\tFragColor = vec4(%s, 1.0);\n}\n";

void ShaderMaker::FormatWithType(std::string& toFormat, InputRef input, std::string firstHalf)
{
    switch (input->type)
    {
    case Type::Float:
        {
            float value = input->GetValue<float>();
            toFormat += FormatString(firstHalf, value);
            break;
        }
    case Type::Int:
        {
            int valueInt = input->GetValue<int>();
            toFormat += FormatString(firstHalf, valueInt);
            break;
        }
    case Type::Bool:
        {
            bool valueBool = input->GetValue<bool>();
            toFormat += FormatString(firstHalf, valueBool);
            break;
        }
    case Type::Vector2:
        {
            Vec2f valueVec2 = input->GetValue<Vec2f>();
            toFormat += FormatString(firstHalf, valueVec2.x, valueVec2.y);
            break;
        }
    case Type::Vector3:
        {
            Vec3f valueVec3 = input->GetValue<Vec3f>();
            toFormat += FormatString(firstHalf, valueVec3.x, valueVec3.y, valueVec3.z);
            break;
        }
    default:
        {
            break;
        }
    }
}

void ShaderMaker::CleanString(std::string& name)
{
    for (char& c : name)
    {
        if (c == ' ')
        {
            c = '_'; // Replace space with underscore
        }
        else if (c == '(' || c == ')')
        {
            c = '\0'; // Mark parentheses for removal
        }
    }
    // Remove marked characters ('\0')
    name.erase(std::remove(name.begin(), name.end(), '\0'), name.end());
}

void ShaderMaker::FillFunctionList(NodeManager* manager, NodeRef firstNode)
{
    FillRecurrence(manager, firstNode);
}

void ShaderMaker::FillRecurrence(NodeManager* manager, const NodeRef& node)
{
    if (node == nullptr) return;

    auto linkManager = manager->GetLinkManager();

    // Recursively process connected nodes (children) first
    for (int i = 0; i < node->p_inputs.size(); i++)
    {
        LinkRef link = linkManager->GetLinkLinkedToInput(node->GetUUID(), i).lock();
        if (link == nullptr)
            continue;

        NodeRef childNode = manager->GetNode(link->fromNodeIndex).lock();
        if (childNode == nullptr)
            continue;

        FillRecurrence(manager, childNode);
    }

    // Process the current node after all its children
    FuncStruct& funcStruct = m_functions[node->p_uuid];
    funcStruct.debugName = node->GetName();

    for (int j = 0; j < node->p_outputs.size(); j++)
    {
        std::string variableName = GetOutputVariableName(node, j);
        funcStruct.outputs.push_back(variableName);
    }

    // Gather input connections
    for (int i = 0; i < node->p_inputs.size(); i++)
    {
        LinkRef link = linkManager->GetLinkLinkedToInput(node->GetUUID(), i).lock();
        if (link == nullptr)
        {
            funcStruct.inputs.emplace_back();
            continue;
        }

        std::shared_ptr<Node> fromNode = manager->GetNode(link->fromNodeIndex).lock();
        if (fromNode == nullptr)
        {
            funcStruct.inputs.emplace_back();
            continue;
        }

        FuncStruct& parentFuncStruct = m_functions[fromNode->p_uuid];
        auto inputName = parentFuncStruct.outputs[link->fromOutputIndex];
        funcStruct.inputs.push_back(inputName);
    }

    if (auto rerouteNode = std::dynamic_pointer_cast<RerouteNodeNamed>(node))
    {
        if (!rerouteNode->IsDefinition())
        {
            if (auto rerouteNodeNamed = NodeRef(rerouteNode->GetDefinitionNode()))
            {
                FillRecurrence(manager, rerouteNodeNamed);
                FuncStruct& parentFuncStruct = m_functions[rerouteNodeNamed->p_uuid];
                auto inputName = parentFuncStruct.outputs[0];
                funcStruct.inputs.push_back(inputName);
            }
        }
    }

    // Add the current node to the serialization list
    m_nodesToSerialize.push_back(node);
}


void ShaderMaker::DoWork(NodeManager* manager)
{
    auto endNode = manager->GetNodeWithName("Material").lock();
    FillFunctionList(manager, endNode);

    std::unordered_map<UUID, FuncStruct> functionList = m_functions;

    for (NodeRef& node : std::views::values(manager->m_nodes))
    {
        if (!node || !node->p_preview)
            continue;
        std::string content;

        CreateFragmentShader(content, manager, node);

        node->m_shader->RecompileFragmentShader(content.c_str());
    }
}

void ShaderMaker::CreateFragmentShader(std::string& content, NodeManager* manager)
{
    // Get all nodes connected to the end node
    NodeRef endNode = manager->GetNodeWithName("Material").lock();

    CreateFragmentShader(content, manager, endNode);
}

void ShaderMaker::CreateFragmentShader(const std::filesystem::path& path, NodeManager* manager)
{
    std::string content;
    CreateFragmentShader(content, manager);

    std::ofstream file(path, std::ios::out | std::ios::trunc);
    file << content;
    file.close();
}

void ShaderMaker::CreateFragmentShader(std::string& content, NodeManager* manager, const NodeRef& endNode)
{
    content.clear();

    FillFunctionList(manager, endNode);

    std::cout << "ShaderMaker::CreateFragmentShader()\n";

    content = s_shaderHeader;
    std::set<std::string> paramNodeDone;
    for (const auto& currentNode : m_nodesToSerialize)
    {
        if (Ref<ParamNode> paramNode = std::dynamic_pointer_cast<ParamNode>(currentNode.lock()))
        {
            if (!paramNode->ShouldSerialize() || paramNodeDone.contains(paramNode->GetParamName()))
                continue;

            paramNodeDone.insert(paramNode->GetParamName());
            content += "uniform " + TypeToGLSLType(paramNode->GetType()) + " " + paramNode->GetParamName() + ";\n";
        }
    }

    std::set<UUID> customNodesDone;
    for (const auto& currentNode : m_nodesToSerialize)
    {
        if (CustomNodeRef customNode = std::dynamic_pointer_cast<CustomNode>(currentNode.lock()))
        {
            if (customNodesDone.contains(customNode->p_uuid))
                continue;
            customNodesDone.insert(customNode->p_uuid);
            content += customNode->GetContent();
        }
    }

    content += s_shaderMainHeader;

    auto templateList = NodeTemplateHandler::GetInstance()->GetTemplates();

    SerializeFunctions(manager, endNode, content);

    FuncStruct& funcStruct = m_functions[endNode->p_uuid];

    content += endNode->ToShader(this, funcStruct);

    bool isMaterial = endNode->GetName() == "Material";
    // If it's not a material node, display only the plane texture on the preview,
    // else display the full shader
    if (!isMaterial)
    {
        content += "\n// Output to screen\n";
        if (!endNode->GetOutputs().empty())
        {
            auto output = endNode->GetOutputs()[0];
            switch (output->type)
            {
            case Type::Float:
            case Type::Int:
            case Type::Bool:
                content += "FragColor = vec4(" + GetOutputVariableName(endNode, 0) + ", 0.0, 0.0, 1.0);\n}\n";
                break;
            case Type::Vector2:
                content += "FragColor = vec4(" + GetOutputVariableName(endNode, 0) + ", 0.0, 1.0);\n}\n";
                break;
            case Type::Vector3:
                content += "FragColor = vec4(" + GetOutputVariableName(endNode, 0) + ", 1.0);\n}\n";
                break;
            default: ;
            }
        }
        else
        {
            // When the node is not the material node
            content += "FragColor = vec4(";
            std::vector<LinkWeakRef> links = endNode->GetLinks();
            std::string outputName;
            if (links.empty())
            {
                outputName = GetValueAsString(endNode->GetInput(0));
            }
            else
            {
                Ref<Link> firstLink = links[0].lock();
                outputName = m_functions[firstLink->fromNodeIndex].outputs[firstLink->fromOutputIndex];
            }
            content += outputName + ", 1.0);\n}\n";
        }
    }
    else
    {
        std::vector<std::string> variableNames;
        // It's the material node, apply the custom shader designed by the user;
        std::string footer = s_shaderFooter;

        std::string toFormat = footer;
        std::string secondHalf = toFormat;
        toFormat.clear();
        for (int j = 0; j < endNode->p_inputs.size(); j++)
        {
            InputRef input = endNode->p_inputs[j];
            size_t findFirst = secondHalf.find_first_of("%");
            size_t index = findFirst + 2;
            if (findFirst == std::string::npos)
                break;
            std::string firstHalf = secondHalf.substr(0, index);
            if (index != std::string::npos)
                secondHalf = secondHalf.substr(index);
            if (!input->isLinked)
            {
                variableNames.push_back(ShaderMaker::GetValueAsString(input));
            }
            auto parentVariableName = funcStruct.inputs[j];
            if (parentVariableName.empty() && !variableNames.empty())
                parentVariableName = variableNames.back();
            toFormat += FormatString(firstHalf, parentVariableName.c_str());
        }
        content += toFormat + secondHalf + "\n";
    }

    m_allVariableNames.clear();
    m_functions.clear();
    m_variablesNames.clear();
    m_nodesToSerialize.clear();
}

void ShaderMaker::CreateShaderToyShader(NodeManager* manager)
{
    // Get all nodes connected to the end node
    NodeRef endNode = manager->GetNodeWithName("Material").lock();

    TemplateList& templateList = NodeTemplateHandler::GetInstance()->GetTemplates();

    std::string content;

    FillFunctionList(manager, endNode);

    std::set<UUID> customNodesDone;
    for (const auto& currentNode : m_nodesToSerialize)
    {
        if (CustomNodeRef customNode = std::dynamic_pointer_cast<CustomNode>(currentNode.lock()))
        {
            if (customNodesDone.contains(customNode->p_uuid))
                continue;
            customNodesDone.insert(customNode->p_uuid);
            content += "void " + customNode->GetFunctionName() + "(";
            for (const auto& input : customNode->p_inputs)
            {
                content += "in " + TypeToGLSLType(input->type) + " " + input->name + ", ";
            }
            for (const auto& output : customNode->p_outputs)
            {
                content += "out " + TypeToGLSLType(output->type) + " " + output->name + ", ";
            }
            content.erase(content.end() - 2, content.end());
            content += ")\n{\n";
            content += customNode->GetContent() + "\n}\n";
        }
    }

    content +=
        "void mainImage( out vec4 fragColor, in vec2 fragCoord )\n{\n// Normalized pixel coordinates (from 0 to 1)\nvec2 uv = fragCoord/iResolution.xy;\n";

    SerializeFunctions(manager, endNode, content);
    content += "\n// Output to screen\nfragColor = vec4(";
    auto firstLink = endNode->GetLinks()[0].lock();
    auto outputName = m_functions[firstLink->fromNodeIndex].outputs[firstLink->fromOutputIndex];
    content += outputName + ", 1.0);\n}\n";

    // TODO
    // ImGui::SetClipboardText(content.c_str());

    std::cout << content;
}

void ShaderMaker::SerializeFunctions(NodeManager* manager, const NodeRef& node, std::string& content)
{
    auto templateList = NodeTemplateHandler::GetInstance()->GetTemplates();
    NodeRef currentNode;
    for (int i = 0; i < m_functions[node->p_uuid].inputs.size(); i++)
    {
        std::string basicStrings = m_functions[node->p_uuid].inputs[i];
        Ref<RerouteNodeNamed> isReroute = std::dynamic_pointer_cast<RerouteNodeNamed>(node);
        if (!isReroute || isReroute && isReroute->IsDefinition())
        {
            LinkRef link = manager->GetLinkManager()->GetLinkLinkedToInput(node->GetUUID(), i).lock();
            if (link == nullptr)
                continue;
            currentNode = manager->GetNode(link->fromNodeIndex).lock();
            if (currentNode == nullptr)
                continue;
        }
        else
        {
            if (!isReroute->IsDefinition())
            {
                currentNode = isReroute->GetDefinitionNode();
            }
        }

        SerializeFunctions(manager, currentNode, content);

        FuncStruct& funcStruct = m_functions[currentNode->p_uuid];

        content += currentNode->ToShader(this, funcStruct);
    }
}

std::string ShaderMaker::ToGLSLVariable(Type type, const Vec4f& value)
{
    switch (type)
    {
    case Type::Float:
        return std::to_string(value.x);
    case Type::Int:
        return std::to_string(static_cast<int>(value.x));
    case Type::Bool:
        return static_cast<bool>(value.x) ? "true" : "false";
    case Type::Vector2:
        return "vec2(" + std::to_string(value.x) + ", " + std::to_string(value.y) + ")";
    case Type::Vector3:
        return "vec3(" + std::to_string(value.x) + ", " + std::to_string(value.y) + ", " + std::to_string(value.z) +
            ")";
    case Type::Vector4:
        return "vec4(" + std::to_string(value.x) + ", " + std::to_string(value.y) + ", " + std::to_string(value.z) +
            ", " + std::to_string(value.w) + ")";
    default:
        return "";
    }
}

std::string ShaderMaker::GetValueAsString(InputRef input)
{
    return ToGLSLVariable(input->type, input->GetValue());
}

std::string ShaderMaker::GetOutputVariableName(NodeRef currentNode, int j)
{
    auto name = currentNode->GetName();
    CleanString(name);
    return name + "_" + std::to_string(currentNode->p_uuid) + "_" + std::to_string(j);
}

std::string ShaderMaker::TypeToGLSLType(Type type)
{
    switch (type)
    {
    case Type::Float:
        return "float";
    case Type::Int:
        return "int";
    case Type::Bool:
        return "bool";
    case Type::Vector2:
        return "vec2";
    case Type::Vector3:
        return "vec3";
    case Type::Vector4:
        return "vec4";
    default:
        return "void";
    }
}
