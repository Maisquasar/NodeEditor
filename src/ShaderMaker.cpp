#include "ShaderMaker.h"

#include "NodeTemplateHandler.h"

template<typename ... Args>
std::string FormatString( const std::string& format, Args ... args )
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

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

void cleanString(std::string& name) {
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

void ShaderMaker::RecurrenceWork(NodeManager* manager, const NodeRef& endNode, TemplateList& templateList, std::string& content,
                                 LinkManager* linkManager, bool insert)
{
    for (int i = 0; i < endNode->p_inputs.size(); i++)
    {
        LinkRef link = linkManager->GetLinkLinkedToInput(endNode->GetUUID(), i).lock();
        if (link == nullptr)
            continue;
        NodeRef node = manager->GetNode(link->fromNodeIndex).lock();
        if (node == nullptr)
            continue;
        
        {
            auto name = node->GetName();
            cleanString(name);
            std::string variableName = name + "_" + std::to_string(node->p_uuid) + "_" + std::to_string(0);
            if (m_allVariableNames.contains(variableName))
                insert = false;
        }
        
        RecurrenceWork(manager, node, templateList, content, linkManager, insert);

        auto it = std::ranges::find_if(templateList, [node](NodeMethodInfo& templateNode) { return templateNode.node->p_templateID == node->GetTemplateID(); });
        const NodeMethodInfo& templateNode = *it;

        std::vector<std::string> variableNames = {};
        for (int k = 0; k < node->p_outputs.size(); k++)
        {
            auto name = node->GetName();
            cleanString(name);
            std::string variableName = name + "_" + std::to_string(node->p_uuid) + "_" + std::to_string(k);
            std::string glslType = TypeToGLSLType(node->p_outputs[k]->type);
            
            std::string thisContent = glslType + " " + variableName + " = ";
            if (m_allVariableNames.contains(variableName))
            {
                thisContent = variableName + " = ";
            }
            if (insert)
                m_allVariableNames.insert(variableName);
            
            std::string toFormat = templateNode.outputFormatStrings[k];
            std::string secondHalf = toFormat;
            toFormat.clear();
            for (int j = 0; j < node->p_inputs.size(); j++)
            {
                InputRef input = node->p_inputs[j];
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
                if (!m_variablesNames.empty())
                {
                    toFormat += FormatString(firstHalf, m_variablesNames.back().c_str());
                    if (k == node->p_outputs.size() - 1)
                        m_variablesNames.erase(m_variablesNames.end() - 1);
                }
            
            }
            thisContent += toFormat + secondHalf + ";\n";
            variableNames.push_back(variableName);

            if (insert)
                content += thisContent;

            std::cout << thisContent << '\n';
        }

        m_variablesNames.insert(m_variablesNames.begin(), variableNames.begin(), variableNames.end());
    }
}

void ShaderMaker::CreateFragmentShader(NodeManager* manager)
{
    // Get all nodes connected to the end node
    NodeRef endNode = manager->GetNodeWithName("Material").lock();

    TemplateList& templateList = NodeTemplateHandler::GetInstance()->GetTemplates();
    
    std::string content;
    auto linkManager = manager->GetLinkManager();

    RecurrenceWork(manager, endNode, templateList, content, linkManager);
    // TODO
    ImGui::SetClipboardText(content.c_str());

    std::cout << content;
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
        return "";
    }
}
