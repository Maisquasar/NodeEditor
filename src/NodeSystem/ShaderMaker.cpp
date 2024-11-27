#include "NodeSystem/ShaderMaker.h"

#include "NodeSystem/CustomNode.h"
#include "NodeSystem/NodeTemplateHandler.h"

#define FOR_SHADER_TOY

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

void ShaderMaker::CleanString(std::string& name) {
    for (char& c : name) {
        if (c == ' ') {
            c = '_'; // Replace space with underscore
        } else if (c == '(' || c == ')') {
            c = '\0'; // Mark parentheses for removal
        }
    }
    // Remove marked characters ('\0')
    name.erase(std::remove(name.begin(), name.end(), '\0'), name.end());
}

void ShaderMaker::FillFunctionList(NodeManager* manager)
{
    auto firstNode = manager->GetNodeWithName("Material").lock();
    FillRecurrence(manager, firstNode, nullptr);

    std::string content;
    for (auto& it : m_functions)
    {
        content += it.second.debugName + " {\n";
        for (int i = 0; i < it.second.inputs.size(); i++)
        {
            if (it.second.inputs[i].empty())
                continue;
            content += '\t' + it.second.inputs[i] + ";\n";
        }
        content += "}\n{\n";
        for (int i = 0; i < it.second.outputs.size(); i++)
        {
            if (it.second.outputs[i].empty())
                continue;
            content +=  '\t' + it.second.outputs[i] + ";\n";
        }
        content += "}\n";
    }
    std::cout << content;
}

void ShaderMaker::FillRecurrence(NodeManager* manager, const NodeRef& node, const NodeRef& parentNode)
{
    auto linkManager = manager->GetLinkManager();
    for (int i = 0; i < node->p_inputs.size(); i++)
    {
        LinkRef link = linkManager->GetLinkLinkedToInput(node->GetUUID(), i).lock();
        if (link == nullptr)
            continue;
        NodeRef currentNode = manager->GetNode(link->fromNodeIndex).lock();
        if (currentNode == nullptr)
            continue;
        
        FuncStruct& funcStruct = m_functions[currentNode->p_uuid];
        funcStruct.debugName = currentNode->GetName();
        
        for (int j = 0; j < currentNode->p_outputs.size(); j++)
        {
            std::string variableName = GetOutputVariableName(currentNode, j);

            funcStruct.outputs.push_back(variableName);
        }

        FillRecurrence(manager, currentNode, node);
        
        m_nodesToSerialize.push_back(currentNode);

        if (node)
        {
            for (int j = 0; j < currentNode->p_inputs.size(); j++)
            {
                LinkRef linkRef = linkManager->GetLinkLinkedToInput(currentNode->p_uuid, j).lock();
                if (linkRef == nullptr)
                {
                    funcStruct.inputs.emplace_back("");
                    continue;
                }
                auto fromNode = manager->GetNode(linkRef->fromNodeIndex).lock();
                FuncStruct parentFuncStruct = m_functions[fromNode->p_uuid];
                auto inputName = parentFuncStruct.outputs[linkRef->fromOutputIndex];
                funcStruct.inputs.push_back(inputName);
            }
        }
    }
}

void ShaderMaker::CreateFragmentShader(NodeManager* manager)
{
    // Get all nodes connected to the end node
    NodeRef endNode = manager->GetNodeWithName("Material").lock();

    TemplateList& templateList = NodeTemplateHandler::GetInstance()->GetTemplates();
    
    std::string content;
    auto linkManager = manager->GetLinkManager();

    FillFunctionList(manager);

    for (const auto& currentNode : m_nodesToSerialize)
    {
        if (CustomNodeRef customNode = std::dynamic_pointer_cast<CustomNode>(currentNode.lock()))
        {
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

#ifdef FOR_SHADER_TOY
    content += "void mainImage( out vec4 fragColor, in vec2 fragCoord )\n{\n// Normalized pixel coordinates (from 0 to 1)\nvec2 uv = fragCoord/iResolution.xy;\n";
#endif
    
    SerializeFunctions(manager, endNode, content);

#ifdef FOR_SHADER_TOY
    content += "\n// Output to screen\nfragColor = vec4(";
    auto firstLink = endNode->GetLinks()[0].lock();
    auto outputName = m_functions[firstLink->fromNodeIndex].outputs[firstLink->fromOutputIndex];
    content += outputName + ", 1.0);\n}\n";
#endif
    
    // TODO
    ImGui::SetClipboardText(content.c_str());

    std::cout << content;
}

void ShaderMaker::SerializeFunctions(NodeManager* manager, const NodeRef& node, std::string& content)
{
#if 0
    auto templateList = NodeTemplateHandler::GetInstance()->GetTemplates();
    for (int i = 0; i < node->p_inputs.size(); i++)
    {
        LinkRef link = manager->GetLinkManager()->GetLinkLinkedToInput(node->GetUUID(), i).lock();
        if (link == nullptr)
            continue;
        NodeRef currentNode = manager->GetNode(link->fromNodeIndex).lock();
        if (currentNode == nullptr)
            continue;

        if (auto customNode = std::dynamic_pointer_cast<CustomNode>(currentNode))
        {
            std::string glslType = TypeToGLSLType(customNode->p_outputs.back()->type);
            content += glslType + " " + customNode->GetFunctionName() + "(";
            for (int i = 0; i < currentNode->p_inputs.size(); i++)
            {
                auto input = currentNode->p_inputs[i];
                content += "in " + TypeToGLSLType(input->type) + " " + input->name + ", ";
            }
            for (int i = 0; i < currentNode->p_outputs.size(); i++)
            {
                auto output = currentNode->p_outputs[i];
                content += "out " + TypeToGLSLType(output->type) + " " + output->name + ", ";
            }
            content.erase(content.end() - 2, content.end());
            content += ")\n{\n";
            content += customNode->GetContent() + "\n}\n";
        }
        
        SerializeFunctions(manager, currentNode, content);

        FuncStruct& funcStruct = m_functions[currentNode->p_uuid];

        auto it = std::ranges::find_if(templateList, [currentNode](NodeMethodInfo& templateNode) { return templateNode.node->p_templateID == currentNode->GetTemplateID(); });
        const NodeMethodInfo& templateNode = *it;

        std::vector<std::string> variableNames = {};
        
        auto toFormatList = currentNode->GetFormatStrings();
        bool isCustomNode = std::dynamic_pointer_cast<CustomNode>(currentNode) != nullptr;
        for (int k = 0; k < currentNode->p_outputs.size(); k++)
        {
            std::string variableName = funcStruct.outputs[k];
            std::string glslType = TypeToGLSLType(currentNode->p_outputs[k]->type);
            
            std::string thisContent = glslType + " " + variableName + " = ";
            if (isCustomNode)
            {
                thisContent = "";
                for (int j = 0; j < currentNode->p_outputs.size(); j++)
                {
                    thisContent += TypeToGLSLType(currentNode->p_outputs[k]->type)
                    + " " + GetOutputVariableName(currentNode, j) + ";\n";
                }
            }
            if (m_allVariableNames.contains(variableName))
            {
                continue;
            }
            m_allVariableNames.insert(variableName);
            
            std::string toFormat = toFormatList[k];
            std::string secondHalf = toFormat;
            toFormat.clear();
            for (int j = 0; j < currentNode->p_inputs.size(); j++)
            {
                InputRef input = currentNode->p_inputs[j];
                size_t index = secondHalf.find_first_of("%") + 2;
                if (index == std::string::npos)
                    break;
                std::string firstHalf = secondHalf.substr(0, index);
                if (index != std::string::npos)
                    secondHalf = secondHalf.substr(index);
                if (!input->isLinked)
                {
                    m_variablesNames.push_back(GetValueAsString(input));
                }
                auto parentVariableName = funcStruct.inputs[j];
                if (parentVariableName.empty())
                    parentVariableName = m_variablesNames.back();
                toFormat += FormatString(firstHalf, parentVariableName.c_str());
            }
            if (isCustomNode)
            {
                for (int j = 0; j < currentNode->p_outputs.size(); j++)
                {
                    size_t index = secondHalf.find_first_of("%") + 2;
                    if (index == std::string::npos)
                        break;
                    std::string firstHalf = secondHalf.substr(0, index);
                    if (index != std::string::npos)
                        secondHalf = secondHalf.substr(index);
                    auto outputName = GetOutputVariableName(currentNode, j);
                    toFormat += FormatString(firstHalf, outputName.c_str());
                }
            }
            thisContent += toFormat + secondHalf + ";\n";
            variableNames.push_back(variableName);

            content += thisContent;

            std::cout << thisContent << '\n';
        }
        
    }
#else
    auto templateList = NodeTemplateHandler::GetInstance()->GetTemplates();
    for (int i = 0; i < node->p_inputs.size(); i++)
    {
        LinkRef link = manager->GetLinkManager()->GetLinkLinkedToInput(node->GetUUID(), i).lock();
        if (link == nullptr)
            continue;
        NodeRef currentNode = manager->GetNode(link->fromNodeIndex).lock();
        if (currentNode == nullptr)
            continue;
        
        SerializeFunctions(manager, currentNode, content);

        FuncStruct& funcStruct = m_functions[currentNode->p_uuid];
        
        content += currentNode->ToShader(this, funcStruct);        
    }
#endif
}

std::string ShaderMaker::GetValueAsString(InputRef input)
{
    switch (input->type)
    {
    case Type::Float:
        return std::to_string(input->GetValue<float>());
    case Type::Int:
        return std::to_string(input->GetValue<int>());
    case Type::Bool:
        return std::to_string(input->GetValue<bool>());
    case Type::Vector2:
        {
            Vec2f value = input->GetValue<Vec2f>();
            return "vec2(" + std::to_string(value.x) + ", " + std::to_string(value.y) + ")";
        }
    case Type::Vector3:
        {
            Vec3f value = input->GetValue<Vec3f>();
            return "vec3(" + std::to_string(value.x) + ", " + std::to_string(value.y) + ", " + std::to_string(value.z) + ")";
        }
    default:
        return "";
    }
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
    default:
        return "void";
    }
}
